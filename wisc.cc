#include "wisc.h"

void wisc_put(WK *wk, string &key, string &value)
{
    if (value.length() <= SELECTIVE_THRESHOLD) // selective KV
    {
        string input = SELECRIVE + value;
        long long offset = wk->head;
        long long size = input.length();

	    lsmt_put(wk->leveldb, key, input); // write to lsmt

        return;
    }

    string input = key + DELIMITER + value + GC_DELIMITER;

    long long offset = wk->head;
    long long size = input.length();

    char ch[size];

    // int gc_flag = 1;

    // while (gc_flag) // GC를 해도 충분한 공간을 만들지 못한 경우
    // {
    //     gc_flag = gc_check(wk, value.length());
    //     if (gc_flag)
    //     {
    //         gc_mech(wk);
    //     }
    // }

    strcpy(ch, input.c_str());
    vlog_write(wk, size, ch);

    string value_addr ;
	string value_size ;
	value_addr = to_string(offset);
	value_size = to_string(size);
	ostringstream lsmStream;
	lsmStream << value_addr << DELIMITER << value_size ; // lsmt string (value_addr, value_size)
	string lsmstr = lsmStream.str();
	lsmt_put(wk->leveldb, key, lsmstr); // write to lsmt
}

bool wisc_get(WK *wk, string &key, string &value)
{
	//cout << "wiscget" << endl;
	string lsmstr;

	const bool found = lsmt_get(wk->leveldb, key, lsmstr);
	if (!found)
		return false;
	string value_addr_str;
	string value_size_str;
	size_t pos = 0;
	string token;
    int num;

    if((lsmstr.find(SELECRIVE)) != string::npos) // selective KV
	{
        lsmstr.erase(0, 1);
        value = lsmstr;

        cout << "selective KV" << endl;
        
        return true;
    }

	if((pos = lsmstr.find(DELIMITER)) != string::npos) // find delimeter
	{
		value_addr_str = lsmstr.substr(0, pos);

		lsmstr.erase(0, pos + DELI_LENGTH);
		value_size_str = lsmstr;
	}
	else
	{
		cout << "lsmt error" << endl ;
		exit(1);	
	}

	long value_addr = stol(value_addr_str);
	long value_size = stol(value_size_str);

    int offset = value_addr;
    string data(value_size, '\0');
    
    // TODO : write이랑 똑같게

    vlog_read(wk, offset, value_size, data);
    
    if ((pos = data.find(DELIMITER)) != string::npos) // find delimeter
    {
        data.erase(0, pos + DELI_LENGTH);

        data.erase(data.end()-2, data.end());

        value = data;
    }
    else
    {
		cout << "vlog error" << endl ;
		exit(1);	
	}

	return true;
}

void vlog_read(WK *wk, long long offset, long long value_size, string &data)
{
    fstream infile(wk->logfile);
    if (FILE_SIZE - (offset % FILE_SIZE) > value_size)
    {
        offset = offset % FILE_SIZE;
        infile.seekg(offset, ios::beg);
        infile.read(&data[0], value_size);
    }
    else
    {
        cout << "else read" << endl;
        // TODO: write 2번으로 끝나게 최적화
        for (int i = 0; i < value_size; i++)
        {
            offset = offset % FILE_SIZE;
            infile.seekg(offset, ios::beg);

            infile.read(&data[i], 1);
            offset++;
        }
    }
    infile.close();
}


void vlog_write(WK *wk, long long size, char *ch)
{
    long long off = wk->head % FILE_SIZE;
    if (FILE_SIZE - off > size) {
        wk->logStream.seekp(off, ios::beg);
        wk->logStream.write(ch, size);
        wk->head += size;
    }
    else
    {
        cout << "else write" << endl;
        // TODO: write 2번으로 끝나게 최적화
        for (int j = 0; j < size; j++)
        {            
            off = wk->head % FILE_SIZE;
            wk->logStream.seekp(off, ios::beg);
            wk->logStream.write(&ch[j], 1);
            wk->head++;
        }
    }

    wk->logStream.flush();
    wk->logStream.sync();
}


// int gc_check(WK *wk, int valuesize)
// {
//     int gc_policy = GC_DEMAND;
//     int remain_space;

//     if (wk->head > wk->tail)
//     {
//         remain_space = FILE_SIZE - (wk->head - wk->tail); // head front, tail back
//     }
//     else
//     {
//         remain_space = wk->tail - wk->head; // head is chasing the tail
//     }
        
//     switch (gc_policy)
//     {
//     case GC_DEMAND:
//         if(remain_space < valuesize)
//         {
//             return 1;
//         }
//         else return 0;
//         break;
    
//     default:
//         break;
//     }
// }

// int gc_mech(WK *wk)
// {
//     int offset = 0; // 0 부터 시작하여 KVpair size 만큼씩 증가
//     char *gc_buff;
//     gc_buff = (char*)malloc(GC_CHUNK_SIZE) // valid data 저장용

//     while (offset < GC_CHUNK_SIZE)
//     {
//         offset = vlog_parser(wk, offset, gc_buff);
//     }


// }

// int vlog_parser(WK *wk, int vlog_offset, char *gc_buff) // vlog의 KVpair을 읽은 뒤 valid check하여 gc_buff에 담아줌, 다음 KVpair로 offset 증가
// {
//     string data(value_size, '\0');
    
//     vlog_read(WK *wk, vlog_offset, )
// }


// int valid_check(string &key, int &offset)
// {
//     string lsmstr;

//     const bool found = lsmt_get(wk->leveldb, key, lsmstr);
// 	if (!found) // 말이 안되는 상황
//     {
//         printf("WTF\n");
//         exit(1);
//     }
//     string value_addr_str;
// 	//string value_size_str;
// 	size_t pos = 0;
// 	string token;
//     int num;

// 	if((pos = lsmstr.find(DELIMITER)) != string::npos) // find delimeter
// 	{
// 		value_addr_str = lsmstr.substr(0, pos);

// 		//lsmstr.erase(0, pos + DELI_LENGTH);
// 		//value_size_str = lsmstr;
// 	}
// 	else
// 	{
// 		cout << "lsmt error" << endl ;
// 		exit(1);	
// 	}

// 	long value_addr = stol(value_addr_str);
// 	//long value_size = stol(value_size_str);

//     return (offset == value_addr);

// }

void timer (bool start = true, const char *label = 0) {
    static chrono::system_clock::time_point startTime;
    if (start) {
        startTime = chrono::system_clock::now();
    } else {
        chrono::system_clock::time_point endTime = chrono::system_clock::now();
        printf("Elapsed Time (%s): %.6lf s\n", label, chrono::duration_cast<chrono::microseconds>(endTime - startTime).count() / 1000.0 / 1000.0);
    }
}

void startTimer() {
    timer(true);
}

void stopTimer(const char *label) {
    timer(false, label);
}

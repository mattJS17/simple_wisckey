#include "wisc.h"

WK_HEAD *wk_head;

int main(int argc, char **argv)
{
    destroy_leveldb("wisckey_test_dir");
    remove("logfile1");
    wk_head = open_wisckey("wisckey_test_dir");
    if (wk_head == NULL)
    {
        cerr << "open failed!" << endl;
        exit(1);
    }

    startTimer();
    for(int j=0; j<100; j++)
    {
        for (unsigned int i = 0; i < 100; ++i)
        {
            ostringstream keyStream;
            keyStream << "Key" << i;

            ostringstream valueStream;
            valueStream << "Test data value: " << i;
            string keystr = keyStream.str();
            string valuestr = valueStream.str();
            wisc_put(wk_head, keystr, valuestr);
    
            ostringstream keyStream1;
            keyStream1 << "Key" << i;

            ostringstream valueStream1;
            valueStream1 << "Test data value: " << i;

            string read_value1;
            string keystr1 = keyStream1.str();

            wisc_get(wk_head, keystr1, read_value1);

            cout << valueStream1.str() << read_value1 << endl;
            if (valueStream1.str() != read_value1)
            {
                cout << "ERROR##########################" << endl;
                exit(1);
            }
        }
    }
    stopTimer("WiscKey run time");

    cout << "Finish Driver" << endl;
    
    return 0;
}

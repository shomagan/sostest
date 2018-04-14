
#include <cstdlib>
#include <sapi/sys.hpp>

#include "DataTest.hpp"
int DataTest::recursive_number = 0;

DataTest::DataTest() : Test("var::Data"){
    recursive_number = 0;

}


bool DataTest::execute_class_api_case(){
    bool result = true;

    if( execute_fill() == false ){
        print_case_message("why:0", "execute_fill()");
        result = false;
    }

    if( execute_alloc() == false ){
        print_case_message("why:0", "execute_alloc()");
        result = false;
    }

    return result;
}

bool DataTest::execute_fill(){
    bool result = true;
    Data data(128);
    char buffer[128];

    memset(buffer, 0, 128);
    data.fill(0);
    if( memcmp(data.data_const(), buffer, 128) != 0 ){
        print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
        result = false;
    }
    data.fill(2);
    if( memcmp(data.data_const(), buffer, 128) == 0 ){
        print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
        result = false;
    }
    data.clear();
    if( memcmp(data.data_const(), buffer, 128) != 0 ){
        print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
        result = false;
    }

    data.fill(0xaa);
    memset(buffer, 0xaa, 128);

    if( memcmp(data.data_const(), buffer, 128) != 0 ){
        print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
        result = false;
    }

    return result;
}


bool DataTest::execute_alloc(){
    bool result = true;
    char temp_string[] = "temp string";
    u16 data_size = 512;
    //test every API to see if it works as expected -- included giving it invalid values
    Data data;
    Data dynamic_data(data_size);
    Data exist_string(temp_string, sizeof(temp_string));

    if(data.data() != 0){
        print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
        result = false;
    }
    if(exist_string.data() == 0 || (exist_string.capacity() != sizeof(temp_string))){
        print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
        result = false;
    }
    if(data.data_const() == 0){
        print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
        result = false;
    }
    if( dynamic_data.data() == 0 ){
        //failed to allocate memory
        print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
        result = false;
    }
    if( dynamic_data.data_const() == 0 || (dynamic_data.capacity() != data_size)){
        //failed to allocate memory
        print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
        result = false;
    }
    data_size--;
    dynamic_data.alloc(data_size, true);
    if( dynamic_data.data_const() == 0 ||
        dynamic_data.data() == 0 ||
        (dynamic_data.capacity() < data_size)){
        //failed to allocate memory
        print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
        result = false;
    }
    data_size--;
    dynamic_data.alloc(data_size, false);
    if( dynamic_data.data_const() == 0 ||
        dynamic_data.data() == 0 ||
        (dynamic_data.capacity() < data_size)){
        //failed to allocate memory
        print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
        result = false;
    }
    data_size--;
    dynamic_data.resize(data_size);
    if( dynamic_data.data_const() == 0 ||
        dynamic_data.data() == 0 ||
        (dynamic_data.capacity() < data_size)){
        //failed to allocate memory
        print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
        result = false;
    }
    if (dynamic_data.set_capacity(data_size-1)!=0){
        //failed to allocate memory
        print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
        result = false;
    }
    if (dynamic_data.set_capacity(data_size+1)!=0){
        //failed to allocate memory
        print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
        result = false;
    }



    return result;
}

bool DataTest::execute_class_performance_case(){
    bool result = true;
    int i;
    for(i=0; i < 5000; i++){
        u32 data_size = rand() & 0xfff; //12 bits is up to 4096
        Data data(data_size);
        if( data.data() == 0 ){
            print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
			result = false;
            break;
        }

        char buffer[data_size];
        memset(buffer, 0xaa, data_size);
        data.fill(0xaa);
        if( memcmp(buffer, data.data_const(), data_size) ){
            print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
			result = false;
        }
        memset(buffer, 0x00, data_size);
		//add clear test
        data.clear();
        if( memcmp(buffer, data.data_const(), data_size) ){
            print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
			result = false;
        }
        if (data_size){
            char* t;
            t = data.cdata();
			//change one byte in data
            if (data_size < data.minimum_size()){
                t[data_size-1] = 0x0e;
            }else{
                t[data.calc_size()-1] = 0x0e;
            }
            if( !memcmp(buffer, data.data_const(), data_size) ){
                print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
				result = false;
            }
        }
    }

    return result;
}



bool DataTest::execute_class_stress_case(){
    bool result = true;
    int i;

    for(i=0; i < 1000; i++){
        //add recursive test
        u8 size_temp = rand()&0x7f;
        Data data(size_temp);
        recursive_number = 0;
        if(!execute_recursive(data)){
            print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
            result = false;
        }
    }

    return result;
}
/*! \details test used for "stress" test in var::String
   * @return false if some test failed true if passed
 */

bool DataTest::execute_recursive(Data data){
    recursive_number++;
    if (data.calc_size() > data.minimum_size()){
        Data data_new(data.calc_size()-1);
        if( data_new.data() == 0 ){
            //failed to allocate memory
            return false;
        }else{
            char fill_temp;
            //remember value
            fill_temp = recursive_number;
            data_new.fill(fill_temp);
            if (!execute_recursive(data_new)){
                return false;
            }
            char buffer[data.calc_size()];
            //recursive value changes after execute_recursive
            memset(buffer, fill_temp, data_new.calc_size());
            if( memcmp(buffer, data_new.data_const(), data_new.calc_size()) ){
                print_case_message("Failed %s:%d", __PRETTY_FUNCTION__, __LINE__);
                return false;
            }
        }
    }
    return 1;
}
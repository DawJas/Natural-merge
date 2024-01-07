#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#define RECORD_LENGHT 30 //max lenght of an record
#define PAGE_SIZE 10 //number of records that fit on one page ( RECORD_LENGHT * PAGE_SIZE bytes)
using namespace std;

class File;
class Record;
class Buffer;

class Record {
public:
	string record;
	int occur = 0;

	Record();

	Record(string record);

	void clear_record();

	void count_occur();
	void out();
};

class File {
public:
	string file_name;

	File();

	File(string file_name);

	void fill_with_records(int ammount_of_records);

	void copy_from_file(const string& file_name, bool fill);

	void copy_to_file(const string& file_name);

	void out_file();
};

class Buffer {
public:
	int current_index = 0;
	int last_record_index = 0;
	int file_offset = 0;
	File file;
	Record* records;

	Buffer(const  File& file);
	~Buffer();

	void out();

	void clear_records();

	Record* get_next_record(bool read = true);

	void write_record(Record record, bool write = true);

	void write_buffer(bool write=true);

	int get_buffer(bool read=true);
};

string get_random_string();
string change_format_into_file(string input);
string format_string(const string& input);
char get_random_char();
void change_format_from_file(string array, Record& record);
void split();
void display_stats();
void input_records();
void input();
bool merge();


struct stats {
	int no_reads = 0;
	int no_writes = 0;
	int no_splits = -1;//program will do 1 split and 1 merge when the file is sorted
	int no_merges = -1;
	bool display = false;//display tapes after every split/merge 
};

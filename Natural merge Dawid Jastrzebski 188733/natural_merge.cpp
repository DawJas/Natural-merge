#include "natural_merge.hpp"

stats global_stats;


Record::Record() {
	this->record = "";
	this->occur = -1;
}

Record::Record(string record) {
	this->record = record;
	count_occur();
}

void Record::clear_record() {
	this->record = "";
	this->occur = -1;
}

void Record::count_occur() {
	unordered_map<char, int> characterCount;

	for (char c : this->record) {
		if (isalpha(c)) {
			char lowercaseChar = tolower(c);
			characterCount[lowercaseChar]++;
		}
	}

	int totalDuplicateCharacters = 0;

	for (const auto& entry : characterCount) {
		if (entry.second > 1) {
			totalDuplicateCharacters += entry.second - 1;
		}
	}

	this->occur = totalDuplicateCharacters;
}

void Record::out() {
	cout << this->record << "   repetitions: " << occur << endl;
}


File::File() {};

File::File(string file_name) {
	this->file_name = file_name;
}

void File::fill_with_records(int ammount_of_records) {
	ofstream out(this->file_name);

	if (!out)
	{
		return;
	}

	while (ammount_of_records--) {
		out << change_format_into_file(get_random_string()) << "\n";
	}
}

void File::copy_from_file(const string& file_name, bool fill) {
	string line;
	ifstream ini_file(file_name);
	ofstream out_file(this->file_name);
	if (ini_file && out_file) {
		while (getline(ini_file, line)) {
			if (fill)line = change_format_into_file(line);
			out_file << line << endl;
		}
	}
	else {
		printf("Cannot read File");
	}
	ini_file.close();
	out_file.close();
}

void File::copy_to_file(const string& file_name) {
	string line;
	ifstream ini_file(this->file_name);
	ofstream out_file(file_name);
	if (ini_file && out_file) {

		while (getline(ini_file, line)) {
			out_file << line << endl;
		}
	}
	else {
		printf("Cannot read File");
	}
	ini_file.close();
	out_file.close();
}

void File::out_file() {
	string line;
	ifstream out_file(this->file_name);
	Record record;
	if (out_file) {

		while (getline(out_file, line)) {
			change_format_from_file(line, record);
			record.out();
		}
	}
	else {
		printf("Cannot read File");
	}
	out_file.close();
}

char get_random_char() {
	return char((rand() % 26 + 97));
}

string get_random_string() {
	size_t size = rand() % RECORD_LENGHT + 1;
	string random_record = "";
	for (size_t i = 0; i < size; i++) {
		random_record.push_back(get_random_char());
	}
	return random_record;
}

Buffer::Buffer(const  File& file) {
	this->file = file;
	this->records = new Record[PAGE_SIZE];
}

Buffer::~Buffer() { delete[] records; }

void Buffer::out() {
	cout << "\t\t BUFFER \n";
	for (int i = 0; i < PAGE_SIZE; i++) {
		if (this->records[i].occur == -1) break;
		this->records[i].out();
	}
	cout << endl;
}

void Buffer::clear_records() {
	for (int i = 0; i < PAGE_SIZE; i++) {
		this->records[i].clear_record();
	}
}

Record* Buffer::get_next_record(bool read) {

	if (current_index == PAGE_SIZE) {
		this->clear_records();
		//get next page from memory
		//return number of records that were read
		last_record_index = get_buffer(read);
		current_index = 0;
	}

	if (current_index == last_record_index) {
		return NULL;
	}

	current_index++;
	return &records[current_index - 1];
}

void Buffer::write_record(Record record, bool write) {
	if (current_index == PAGE_SIZE) {
		write_buffer(write);
		this->clear_records();
		current_index = 0;
	}
	this->records[current_index] = record;
	current_index++;
}

//writes the whole buffer into a file
void Buffer::write_buffer(bool write) {
	ofstream out(this->file.file_name, std::fstream::app);

	if (!out)
	{
		cout << "Cannot open the file !\n";
	}

	for (int i = 0; i < PAGE_SIZE && records[i].occur != -1; i++) {
		int j = 0;
		while (this->records[i].record.size() + j < RECORD_LENGHT) {
			out << '0';
			j++;
		}
		out << records[i].record << "\n";
	}

	if(write)global_stats.no_writes++;
	out.close();
}

//fills buffer from file
int Buffer::get_buffer(bool read) {
	ifstream in(this->file.file_name, std::fstream::in);
	if (!in) {
		cout << "Cannot open the file!\n";
		return -1;
	}

	if(read)global_stats.no_reads++;
	in.seekg(file_offset); //open the file where you closed it last

	int i = 0;
	string input;
	while (i < PAGE_SIZE && getline(in, input)) {
		change_format_from_file(input, this->records[i]);
		i++;
	}

	file_offset = in.tellg(); // Update the file offset
	return i;
}



void change_format_from_file(string input, Record& record) {
	string temp;
	for (int i = 0; i < RECORD_LENGHT; i++) {
		if (input[i] == '0') continue;
		temp.push_back(input[i]);
	}
	record.record = temp;
	record.count_occur();
}

string change_format_into_file(string input) {
	string empty;
	int ammount_of_zeros = RECORD_LENGHT - input.size();
	while (ammount_of_zeros--) {
		empty += '0';
	}
	empty = empty + input;
	return empty;
}

//split records from tape A to tapes B and C
void split() {

	global_stats.no_splits++;
	File file1("1.txt");
	File file2("2.txt");
	File file3("3.txt");

	Buffer a(file1);
	Buffer b(file2);
	Buffer c(file3);

	remove("2.txt");
	remove("3.txt");

	//get records into the buffer
	a.last_record_index = a.get_buffer();

	int previous_value = 0;
	Buffer* temp = &b;
	Record* temp_record = NULL;

	while (true) {
		temp_record = a.get_next_record();
		if (temp_record == NULL) break;

		if (previous_value > temp_record->occur) {
			if (temp == &b) temp = &c;
			else temp = &b;
		}
		temp->write_record(*temp_record);
		previous_value = temp_record->occur;
	}
	b.write_buffer();
	c.write_buffer();


	if (global_stats.display) {
		cout << "\n\n\t\t SPLIT NO. " << global_stats.no_splits << " :\n";
		cout << "\n\n\t\t TAPE B:\n\n";
		file2.out_file();
		cout << "\n\n\t\t TAPE C:\n\n";
		file3.out_file();
	}

	remove("1.txt");
}

void display_stats() {
	cout << "\nNO. MERGES: " << global_stats.no_merges;
	cout << "\nNO. SPLITS: " << global_stats.no_splits;
	cout << "\nNO. READS: " << global_stats.no_reads;
	cout << "\nNO. WRITES: " << global_stats.no_writes;

}

//merge records from tapes B and C into tape A
//return true if file is already sorted
bool merge() {
	global_stats.no_merges++;
	File fileA("1.txt");
	File fileB("2.txt");
	File fileC("3.txt");

	Buffer bufferA(fileA);
	Buffer bufferB(fileB);
	Buffer bufferC(fileC);

	bufferB.last_record_index = bufferB.get_buffer();
	bufferC.last_record_index = bufferC.get_buffer();

	Record* recordB = bufferB.get_next_record();
	Record* recordC = bufferC.get_next_record();
	int temp_value_B = 0;
	int temp_value_C = 0;


	if (recordC == NULL) {
		fileA.copy_from_file("2.txt", false);
		cout << "\n\n\t\t MERGE NO. " << global_stats.no_merges << " :\n\n";
		fileA.out_file();
		return true;
	}

	while (recordB != nullptr || recordC != nullptr) {
		int occurB = (recordB != nullptr) ? recordB->occur : INT_MAX;
		int occurC = (recordC != nullptr) ? recordC->occur : INT_MAX;

		if (occurB < temp_value_B) {
			while (recordC != nullptr && recordC->occur >= temp_value_C) {
				bufferA.write_record(*recordC);
				temp_value_C = recordC->occur;
				recordC = bufferC.get_next_record();
			}
			temp_value_B = 0;
			temp_value_C = 0;
		}
		else if (occurC < temp_value_C) {
			while (recordB != nullptr && recordB->occur >= temp_value_B) {
				bufferA.write_record(*recordB);
				temp_value_B = recordB->occur;
				recordB = bufferB.get_next_record();
			}
			temp_value_B = 0;
			temp_value_C = 0;
		}
		else {
			if (occurB < occurC) {
				bufferA.write_record(*recordB);
				temp_value_B = occurB;
				recordB = bufferB.get_next_record();
			}
			else {
				bufferA.write_record(*recordC);
				temp_value_C = occurC;
				recordC = bufferC.get_next_record();
			}
		}
	}

	bufferA.write_buffer();

	if (global_stats.display) {
		cout << "\n\t MERGE NO. " << global_stats.no_merges << " :\n";
		fileA.out_file();
	}

	remove("2.txt");
	remove("3.txt");

	return false;
}

string format_string(const string& input) {
	string formated;
	for (char c : input) {
		if (isalpha(c)) {
			formated.push_back(tolower(c));
		}
		else {
			formated.push_back(get_random_char());
		}
	}

	if (formated.size() < 30) {

		string empty;
		while (empty.size() + formated.size() != RECORD_LENGHT) {
			empty.push_back('0');
		}
		formated = empty + formated;
	}

	else {
		string empty;
		int z = 0;
		while (empty.size() != RECORD_LENGHT) {
			empty.push_back(formated.at(z));
			z++;
		}
		formated = empty;
	}
	return formated;
}


void input_records() {
	size_t desired_size;
	File file("hand.txt");
	Buffer buffer(file);
	cout << "\nHow many records would you like input? \n\n\t: ";
	cin >> desired_size;
	cout << "\nInput " << desired_size << " records, they may consist only of letters, max " << RECORD_LENGHT << " characters\n";
	cout << "All other characters will be turned into random letters.\n";

	for (size_t i = 0; i < desired_size; i++) {
		string input_record;
		cin >> input_record;

		string formated = format_string(input_record);
		Record record(formated);
		buffer.write_record(record,false);
	}
	buffer.write_buffer(false);
	file.copy_to_file("1.txt");
}


void input() {
	char input = '0';
	cout << "Dawid Jastrzebski s188733\n";
	cout << "\n\nProgram used to natural sort sequential files";
	cout << "\n Select mode: \n\t[ 1 ] Random records \n\t[ 2 ] Records from file \n\t[ 3 ] Input record from console\n\n";

	remove("1.txt");
	remove("2.txt");
	remove("3.txt");
	remove("hand.txt");

	bool end_switch = false;
	cin >> input;

	while (!end_switch) {
		while (!end_switch) {
			switch (input)
			{
			case '1':
			{
				cout << "\nHow many records would you like to get sorted? \n\n\t: ";
				int no_records;
				cin >> no_records;

				File file("1.txt");
				file.fill_with_records(no_records);
				cout << "\nRecords succesfully generated\n\n";
				file.out_file();
				end_switch = true;
				break;
			}
			case '2':
			{
				cout << "Input filename: \n\n\t: ";
				string output;
				cin >> output;
				File file("1.txt");
				file.copy_from_file(output, true);
				end_switch = true;
				break;
			}
			case '3':
			{
				input_records();
				end_switch = true;
				break;
			}
			default: {
				cout << "Wrong input, please try again.\n";
				cin >> input;
			}
			}
		}

		cout << "\n\t Would you like to display records after every merge? \n\t\t[ 1 ] - Yes \t\t[ 0 ] - No\n";
		cin >> input;
		if (input == '1') global_stats.display = true;
	}
}



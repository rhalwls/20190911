#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<map>
using namespace std;
string global_output;

class BTree {
public:
	int block_size;
	int n_four_byte_entry;
	fstream fin;
	ofstream fout;
	int MAX_KEY = 2147483647;
	BTree() {

	}
	BTree(string fileName, int blockSize) {
		fin.open(fileName, ios::in|ios::out | ios::binary);
		//out.open(fileName, ios::out| ios::app | ios::binary);
		block_size = blockSize;
		int t = (block_size - 4) / 4;
		n_four_byte_entry = 1 + t;

		fout.open(global_output, ios::out);
		if (fout.fail()) {
			cout << "failed to opend output file" << endl;

		}
		
	}




	void get_every_data() {
		int rootid = get_root_id();
		int depth = get_depth();
		int level = 0;
		int current_bid = rootid;
		while (level < depth) {
			int* block = read_a_block(current_bid);
			current_bid = block[0];
			level++;
		}
		int ctr_keys = 0;
		while (current_bid != MAX_KEY) {
			//cout << "block id = " << current_bid << endl;
			int* block = read_a_block(current_bid);
			for (int i = 0; i < n_four_byte_entry; i++) {
				if (i % 2 == 0&&block[i]!=MAX_KEY&&i!=n_four_byte_entry-1) {
					ctr_keys++;
				}
				//cout << block[i] << " ";
			}
			//cout << endl;
			current_bid = block[n_four_byte_entry - 1];
		}
		cout << "inserted key 개수  " << ctr_keys;
	}


	void update_depth() {
		int current_depth = get_depth()+1;

		fin.seekp(8, ios::beg);
		fin.write(reinterpret_cast<const char *>(&current_depth), sizeof(int));
	}
	void insert(int key, int value) {
		if (get_root_id() == 0) {
			//first insertion
			//cout << get_root_id() << endl;
			update_rootid(1);
			int* arr = new int[n_four_byte_entry];
			for (int i = 0; i < n_four_byte_entry; i++) {
				arr[i] = MAX_KEY;
			}
			write_a_block(1,arr);
		}
		go_down_tree(key, value);
	}
	void print() {
		int root_id = get_root_id();
		int* root_block = read_a_block(root_id);
		int current;

		vector<int> children_bid;
		vector<int> level0;
		vector<int> level1;
		for (int i = 0; i < n_four_byte_entry; i++) {
			current = root_block[i];

			if (current == MAX_KEY) {
				//don't print
				break;
			}
			else {
				if (i % 2 == 0) {
					children_bid.push_back(current);
				}
				level0.push_back(current);//current의 (맥스키 제외) 모든 값들 들어가있음
			}
		}
		if (get_depth() != 0) { //maybe change see depth
			for (int i = 0; i < children_bid.size(); i++) {
				int* child = read_a_block(children_bid[i]);
				for (int i = 0; i < n_four_byte_entry; i++) {
					current = child[i];
					if (current == MAX_KEY) {
						//don't print
						break;
					}
					else {
						level1.push_back(current);
					}
				}
			}
		}
		

		if (fout.fail()) {
			cout << "failed to open" << endl;
		}
		fout << "<0>" << endl;
		if (get_depth() == 0) {
			//레벨0이 리프인 경우
			//key는 짝수 인덱스에만
			for (int i = 0; i < level0.size(); i++) {
				if(i%2==0)
				fout << level0[i] << ", ";
			}
			fout << endl;
		}
		else {
			//레벨0이 internal node인 경우
			//key는 홀수 인덱스에만
			for (int i = 0; i < level0.size(); i++) {
				if(i%2 ==1)
				fout << level0[i] << ", ";
			}
			fout << endl;
		}

		fout << "<1>" << endl;
		if (get_depth() == 1) {
			for (int i = 0; i < level1.size(); i++) {
				if(i%2==0)
				fout << level1[i] << ", ";
			}
			fout << endl;
		}
		else {
			for (int i = 0; i < level1.size(); i++) {
				if(i%2 ==1)
				fout << level1[i] << ", ";
			}
			fout << endl;
		}
		
	}
	int search(int key) {
		//cout << "godowntree" << endl;
		int current_depth = get_depth();
		int current_bid = get_root_id();
	


		for (int level = 0; level <= current_depth; level++) {
			if (level == current_depth) {
				//leaf

				//cout << "searched leafnode node# is " << current_bid << endl;
				int* found_node = read_a_block(current_bid);
				print_node(found_node);
				int found_value=-1;
				int keys = (block_size - 4) / 8;
				bool found = false;
				for (int i = 0; i < keys; i++) {
					if (found_node[i * 2] == key) {
						found = true;
						found_value=found_node[i*2+1];
						break;
					}
				}

				if (found) {
					//cout << "searching key was " << key << " found value is " << found_value << endl;
					fout << key << "," << found_value << endl;

				}
				else {
					//cout << "failed to find" << endl;
				}
				return found_value;
				

			}
			else {
				//bid ancestor 수정
				int *buffer = read_a_block(current_bid);
				for (int i = 1; i <= n_four_byte_entry - 2; i += 2) {
					if (buffer[i] > key) {
						current_bid = buffer[i - 1];
						break;
					}
					else if (i == n_four_byte_entry - 2 && key >= buffer[i]) {
						current_bid = buffer[i + 1];
						break;
					}
				}
				
			}

		}
	
	} // point search
	void search(int startRange, int endRange) { // range search
		//cout << "godowntree" << endl;
		int current_depth = get_depth();
		int current_bid = get_root_id();



		for (int level = 0; level <= current_depth; level++) {
			if (level == current_depth) {
				//leaf
				//cout << "range search" << endl;
				while (current_bid != MAX_KEY) {
					//cout << "block id = " << current_bid << endl;
					int* block = read_a_block(current_bid);
					int keys = (block_size - 4) / 8;
					for (int i = 0; i < keys; i++) {
						//i => key
						//i+1 => value
						if (block[2*i] >= startRange && block[2*i] <= endRange) {
							//insert entry
							fout << block[2*i] << "," << block[2*i + 1] << "\t";
							//cout << "key : " << block[2*i] << " value : " << block[2*i + 1] << endl;
						}
					}

					current_bid = block[n_four_byte_entry - 1];
				}

			}
			else {
				//bid ancestor 수정
				int *buffer = read_a_block(current_bid);
				for (int i = 1; i <= n_four_byte_entry - 2; i += 2) {
					if (buffer[i] > startRange) {
						current_bid = buffer[i - 1];
						break;
					}
					else if (i == n_four_byte_entry - 2 && startRange >= buffer[i]) {
						current_bid = buffer[i + 1];
						break;
					}
				}

			}

		}
		fout << endl;
	}

	void print_node(int *arr) {
		for (int i = 0; i < n_four_byte_entry; i++) {
			//cout << arr[i] << " " << endl;
		}
	}
	
	int allocate_new_bid_write_empty_blcok() {
		fin.seekg(0, ios::beg);
		const auto begin = fin.tellg();
		fin.seekg(0, ios::end);
		const auto end = fin.tellg();
		const auto fsize = (end - begin);
		//cout << "마지막 노드의 bid는 " << (fsize - 12) / block_size << endl;

		int newid = (fsize - 12) / block_size + 1;
		int* emptyblock = new int[n_four_byte_entry];
		for (int i = 0; i < n_four_byte_entry; i++) {
			emptyblock[i] = MAX_KEY;
		}
		write_a_block(newid, emptyblock);

		return newid;
	}
	pair<int, int> insert_at_internal_node(int bid, int key, int value) {
		int* current_node = read_a_block(bid);
		print_node(current_node);

		if (check_node_full(bid)) {
			//cout << "internal need to split" << endl;
			//need to split
			int *copy_current_node = new int[n_four_byte_entry + 2];
			int copyidx = 0;

			

			bool inserted = false;
			int keys_in_a_block = (block_size - 4) / 8;
			map<int, int> temp;
			temp.insert(make_pair(key, value));
			for (int i = 0; i < keys_in_a_block; i++) {
				int temp_key = current_node[2 * i + 1];
				int temp_value = current_node[2 * i + 2];
				temp.insert(make_pair(temp_key,temp_value));
			}
			copy_current_node[0] = current_node[0];
			int i = 1;
			for (map<int, int>::iterator it = temp.begin(); it != temp.end(); it++) {
				copy_current_node[i++] = it->first;
				copy_current_node[i++] = it->second;
			}


			//cout << "copy array" << endl;
			for (int i = 0; i < n_four_byte_entry + 2; i++) {
				//cout << copy_current_node[i] << " ";
			}

			int* a = new int[n_four_byte_entry];//block a는 bid가 기존 bid
			int* b = new int[n_four_byte_entry];//block b는 bid가 새 것

			for (int i = 0; i < n_four_byte_entry; i++) {
				a[i] = MAX_KEY;
				b[i] = MAX_KEY;
			}
			int b_bid = allocate_new_bid_write_empty_blcok();//b의 bid 할당
			
			
			
			//split the huge node
			int huge_node_keys = (block_size - 4)/8+1;
			int riseup;
			a[0] = current_node[0];
			int aidx = 1, bidx = 0;
			for (int i = 0; i < huge_node_keys; i++) {
				if (i == huge_node_keys / 2) {
					riseup = copy_current_node[i*2+1];
					b[bidx++] = copy_current_node[i * 2 + 2];
				
				}
				else if (i < (huge_node_keys / 2)) {
					a[aidx] = copy_current_node[i * 2+1];
					a[aidx + 1] = copy_current_node[i * 2 + 2];
					aidx += 2;
				}
				else {
					//cout << "b에 쓰는중~" << copy_current_node[i * 2+1] << " " << copy_current_node[i * 2 + 2] << endl;
					b[bidx] = copy_current_node[i * 2+1];
					b[bidx + 1]= copy_current_node[i * 2 + 2];
					bidx += 2;
				}
			}
			//cout << "this will be rise up from an internal node" << riseup << endl;
			//b[n_four_byte_entry-1] = copy_current_node[n_four_byte_entry + 2 - 1];
			write_a_block(bid, a);
			write_a_block(b_bid, b);
			if (get_root_id() == bid) {
				//지금 split되는 노드가 root였음
				//bid할당 : 데이터의 마직막 위치(사이즈)를 통해 마지막으로 만든 노드의 bid
				//를 얻고 그것보다 1큰 것을 할당후 rootid 변경
				update_depth();
				//cout << "새 depth " << get_depth() << endl;;
				int new_rootid = allocate_new_bid_write_empty_blcok();
				update_rootid(new_rootid);
				int* new_root_block = read_a_block(new_rootid);
				new_root_block[0] = bid;
				write_a_block(new_rootid, new_root_block);
				insert_at_internal_node(new_rootid, riseup, b_bid);
				
				return pair<int, int>(-1, -1);
			}
			else {
				return pair<int, int>(riseup, b_bid);
				//godowntree에서 받아 부모에게 넣어줌
			}




			delete[] a;
			delete[] b;
			delete[] copy_current_node;
		}
		else {
			//no need to split
			//기존 bid에 데이터 더해 덮어씀
			//cout << "internal no need to split" << endl;
			int *copy_current_node = new int[n_four_byte_entry];
			int copyidx = 1;

			bool inserted = false;

			copy_current_node[0] = current_node[0];
			for (int i = 1; i <= n_four_byte_entry - 4; i += 2) {
				if (current_node[i] > key && !inserted) {

					copy_current_node[copyidx] = key;
					copy_current_node[copyidx + 1] = value;
					copyidx += 2;
					inserted = true;
				}
				copy_current_node[copyidx] = current_node[i];
				copy_current_node[copyidx + 1] = current_node[i + 1];
				copyidx += 2;
			}
			if (!inserted) {
				copy_current_node[copyidx] = key;
				copy_current_node[copyidx + 1] = value;
			}
			//cout << "copy array" << endl;
			for (int i = 0; i < n_four_byte_entry; i++) {
				//cout << copy_current_node[i] << " ";
			}
			//cout << endl;

			write_a_block(bid, copy_current_node);
			delete[] copy_current_node;
			return pair<int, int>(-1, -1);
		}
		delete[] current_node;
	}
	

	pair<int,int> insert_at_leaf_node(int bid, int key, int value) {
		int* current_node = read_a_block(bid);
		print_node(current_node);

		if (check_node_full(bid)) {
			//need to split
			int *copy_current_node = new int[n_four_byte_entry+2];
			int copyidx = 0;
			
			bool inserted = false;
			for (int i = 0; i < n_four_byte_entry - 2; i += 2) {
				if (current_node[i] > key && !inserted) {

					copy_current_node[copyidx] = key;
					copy_current_node[copyidx + 1] = value;
					copyidx += 2;
					inserted = true;
				}
				copy_current_node[copyidx] = current_node[i];
				copy_current_node[copyidx + 1] = current_node[i + 1];
				copyidx += 2;
			}
			if (!inserted) {
				copy_current_node[copyidx] = key;
				copy_current_node[copyidx + 1] = value;
			}
			copy_current_node[n_four_byte_entry+2 - 1] = current_node[n_four_byte_entry - 1];
			
			//cout << "copy array" << endl;
			for (int i = 0; i < n_four_byte_entry+2; i++) {
				//cout << copy_current_node[i] << " ";
			}

			int* a = new int[n_four_byte_entry];//block a는 bid가 기존 bid
			int* b = new int[n_four_byte_entry];//block b는 bid가 새 것
			for (int i = 0; i < n_four_byte_entry; i++) {
				a[i] = MAX_KEY;
				b[i] = MAX_KEY;
			}
			int b_bid = allocate_new_bid_write_empty_blcok();//b의 bid 할당
			int aidx = 0;
			int bidx = 0;
			int riseupkey;
			
			int keys = (block_size - 4) / 8;
			int huge_node_keys = keys + 1;
			for (int i = 0; i < huge_node_keys; i++) {
				if (i == huge_node_keys / 2) {
					riseupkey = copy_current_node[2*i];
					b[bidx++] = copy_current_node[2 * i];
					b[bidx++] = copy_current_node[2 * i + 1];
				}
				else if (i < huge_node_keys / 2) {
					a[aidx++] = copy_current_node[2 * i];
					a[aidx++] = copy_current_node[2 * i + 1];
				}
				else {
					b[bidx++] = copy_current_node[2 * i];
					b[bidx++] = copy_current_node[2 * i + 1];
				}
			}
			a[n_four_byte_entry - 1] = b_bid;
			b[n_four_byte_entry - 1] = current_node[n_four_byte_entry - 1];

			riseupkey =b[0];
			//cout << "this will be rise up from the leaf " << b[0] << endl;
			a[n_four_byte_entry-1] = b_bid;
			b[n_four_byte_entry - 1] = current_node[n_four_byte_entry - 1];
			write_a_block(bid, a);
			write_a_block(b_bid, b);
			if (get_root_id() == bid) {
				//지금 split되는 노드가 root였음
				//bid할당 : 데이터의 마직막 위치(사이즈)를 통해 마지막으로 만든 노드의 bid
				//를 얻고 그것보다 1큰 것을 할당후 rootid 변경
				update_depth();
				int new_rootid = allocate_new_bid_write_empty_blcok();
				update_rootid(new_rootid);
				int* new_root_block = read_a_block(new_rootid);
				new_root_block[0] = bid;
				write_a_block(new_rootid, new_root_block);
				insert_at_internal_node(new_rootid, riseupkey, b_bid);
				return pair<int, int>(-1, -1);
			}
			else {
				return pair<int, int>(riseupkey, b_bid);
				//godowntree에서 받아 부모에게 넣어줌
			}




			delete[] a;
			delete[] b;
			delete[] copy_current_node;
		}
		else {
			//no need to split
			//기존 bid에 데이터 더해 덮어씀
			int *copy_current_node = new int[n_four_byte_entry];
			int copyidx = 0;

			bool inserted = false;
			for (int i = 0; i < n_four_byte_entry-4; i+=2) {
				if (current_node[i] > key&&!inserted) {
					
					copy_current_node[copyidx] = key;
					copy_current_node[copyidx + 1] = value;
					copyidx += 2;
					inserted = true;
				}
				copy_current_node[copyidx] = current_node[i];
				copy_current_node[copyidx + 1] = current_node[i + 1];
				copyidx += 2;
			}
			if (!inserted) {
				copy_current_node[copyidx] = key;
				copy_current_node[copyidx + 1] = value;
			}
			copy_current_node[n_four_byte_entry - 1] = current_node[n_four_byte_entry - 1];
			//cout << "copy array" << endl;
			for (int i = 0; i < n_four_byte_entry; i++) {
				//cout << copy_current_node[i]<<" ";
			}
			//cout << endl;

			write_a_block(bid, copy_current_node);
			delete[] copy_current_node;
			return pair<int, int>(-1, -1);
		}
		delete[] current_node;
	}
	bool check_node_full(int bid) {
		int checking;
		fin.seekg(bid*block_size+4, ios::beg);//errorneous
		fin.read(reinterpret_cast<char *>(&checking), sizeof(int));

		return checking != MAX_KEY;
	}
	
	

	void go_down_tree(int key,int value) {
		//cout << "godowntree" << endl;
		int current_depth = get_depth();
		int current_bid = get_root_id();
		vector<int> ancestors; //루트부터 마지막 리프노드 bid까지
		ancestors.push_back(current_bid);
		for (int level = 0; level <= current_depth; level++) {
			if (level == current_depth) {
				//leaf
				//cout << "leaf에 도달했습니다." << endl;


				pair<int,int> res = insert_at_leaf_node(current_bid, key, value);
				
				for (int i = ancestors.size() - 2; i >= 0; i--) {
					if (res.first == -1) {
						break;
					}
					res = insert_at_internal_node(ancestors[i], res.first, res.second);						
				}
			
			}
			else {
				//bid ancestor 수정
				int *buffer = read_a_block(current_bid);
				for (int i = 1; i <= n_four_byte_entry-2; i += 2) {
					if (buffer[i] > key) {
						current_bid = buffer[i - 1];
						break;
					}
					else if (i == n_four_byte_entry - 2 && key >= buffer[i]) {
						current_bid = buffer[i + 1];
						break;
					}
				}
				ancestors.push_back(current_bid);
			}
			
		}
	}
	int get_depth() {
		int ret;
		fin.seekg(8, ios::beg);
		fin.read(reinterpret_cast<char *>(&ret), sizeof(int));
		return ret;
	}

	void get_blocksize() {
		fin.seekg(0, ios::beg);
		fin.read(reinterpret_cast<char *>(&block_size), sizeof(int));
		int t = (block_size - 4)/4;
		n_four_byte_entry = 1+t;
	}
	int get_root_id() {
		int ret;
		fin.seekg(4, ios::beg);
		fin.read(reinterpret_cast<char *>(&ret), sizeof(int));
		cout << "root id is " << ret << endl;
		return ret;
	}

	bool is_root(int bid) {
		return get_root_id() == bid;
	}


	void write_a_block(int bid,int *arr) {
		fin.seekp(12 + (bid - 1)*block_size);
		print_node(arr);
		cout << "writing a block" << endl;
		for (int i = 0; i < n_four_byte_entry; i++) {
			fin.write(reinterpret_cast<char *>(&arr[i]), sizeof(int));
		}
		//delete[] arr;
	}
	void update_rootid(int bid) {
		fin.seekp(4, ios::beg);
		fin.write(reinterpret_cast<const char *>(&bid), sizeof(int));
	}

	int* read_a_block(int bid) {
		//need to delete the array after use!!!!!!
		fin.seekg(12 + (bid - 1)*block_size);

		int* buffer = new int[n_four_byte_entry];
		for (int i = 0; i < n_four_byte_entry; i++) {
			fin.read(reinterpret_cast<char *>(&buffer[i]), sizeof(int));
			
		}
		return buffer;
	}

};



// Test
int main(int argc, char* argv[]) {
	char command = argv[1][0];
	
	BTree myBtree = BTree();
	string bin_file_name;
	string record_txt_file;
	string output_txt_file;
	ofstream fout;
	ifstream finforinput;
	string tobesplited;
	string skey;
	string sval;
	string sstart, send;

	int start, end;
	int block_size;
	int zero = 0;
	int inserted_key, inserted_value;
	ifstream fin;
	int rootid;
	int depth;
	int ctr_input = 0;

	switch (command) {
	case 'c':
		// create index file

		bin_file_name = string(argv[2]);
		block_size = atoi(argv[3]);
		cout << block_size << endl;

		//binary로 파일을 읽고, 쓴다.
		fout.open(bin_file_name, ios::out | ios::binary);

		fout.write(reinterpret_cast<const char *>(&block_size), sizeof(int));
		fout.write(reinterpret_cast<const char *>(&zero), sizeof(int));
		fout.write(reinterpret_cast<const char *>(&zero), sizeof(int));

		break;
	case 'i':
		// insert records from [records data file], ex) records.txt
		bin_file_name = string(argv[2]);
		record_txt_file = string(argv[3]);
		
		fin.open(bin_file_name, ios::in | ios::binary);
		fin.seekg(0, ios::beg);
		fin.read(reinterpret_cast<char *>(&block_size), sizeof(int));
		myBtree = BTree(bin_file_name, block_size);
		
		finforinput.open(record_txt_file, ios::in);
		int key, val;

		
		while (finforinput>>tobesplited) {
			ctr_input++;
			skey = tobesplited.substr(0, tobesplited.find_first_of(","));
			key = stoi(skey);
			sval = tobesplited.substr(tobesplited.find_first_of(",") + 1);
			val = stoi(sval);

			cout << key <<" "<< val << endl;

			myBtree.insert(key, val);

		}
		cout << "totla " << ctr_input << " entry was inserted" << endl;
		break;
	case 's':
		// search keys in [input file] and print results to [output file]

		bin_file_name = string(argv[2]);
		record_txt_file = string(argv[3]);
		output_txt_file = string(argv[4]);
		global_output = output_txt_file;

		fin.open(bin_file_name, ios::in | ios::binary);
		fin.seekg(0, ios::beg);
		fin.read(reinterpret_cast<char *>(&block_size), sizeof(int));
		myBtree = BTree(bin_file_name, block_size);
		cout << "s" << bin_file_name << record_txt_file << endl;
		
		finforinput.open(record_txt_file, ios::in);
		if (finforinput.fail())            //is it ok?
		{
			cout << "Input file did not open please check it\n";
			system("pause");
			return 1;
		}
		//fout.open(output_txt_file, ios::out);
		
		while (finforinput >> key) {

			fout<<myBtree.search(key)<<endl;

		}
		break;
	case 'r':
		// search keys in [input file] and print results to [output file]

		bin_file_name = string(argv[2]);
		record_txt_file = string(argv[3]);
		output_txt_file = string(argv[4]);
		global_output = output_txt_file;
		fin.open(bin_file_name, ios::in | ios::binary);
		fin.seekg(0, ios::beg);
		fin.read(reinterpret_cast<char *>(&block_size), sizeof(int));
		myBtree = BTree(bin_file_name, block_size);



		finforinput.open(record_txt_file, ios::in);

		if (finforinput.fail())            //is it ok?
		{
			cout << "Input file did not open please check it\n";
			system("pause");
			return 1;
		}

		while (finforinput >> tobesplited) {
			cout << "reading range" << endl;
			ctr_input++;
			sstart = tobesplited.substr(0, tobesplited.find_first_of(","));
			start = stoi(sstart);
			send = tobesplited.substr(tobesplited.find_first_of(",") + 1);
			end = stoi(send);

			cout << start << " " << end << endl;

			myBtree.search(start, end);

		}

		break;
	case 'p':
		// print B+-Tree structure to [output file]
		bin_file_name = string(argv[2]);
		output_txt_file = string(argv[3]);
		global_output = output_txt_file;
		fin.open(bin_file_name, ios::in | ios::binary);
		fin.seekg(0, ios::beg);
		fin.read(reinterpret_cast<char *>(&block_size), sizeof(int));
		myBtree = BTree(bin_file_name, block_size);
		
		myBtree.print();


		break;
	case 'g':
		cout << "print every leaf node" << endl;
		bin_file_name = string(argv[2]);
		fin.open(bin_file_name, ios::in | ios::binary);
		fin.seekg(0, ios::beg);
		fin.read(reinterpret_cast<char *>(&block_size), sizeof(int));
		myBtree = BTree(bin_file_name, block_size);

		myBtree.get_every_data();
		break;
	}

}
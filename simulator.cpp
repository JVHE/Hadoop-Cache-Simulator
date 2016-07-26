#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
using namespace std;

class Namenode;
class File;
class Data;
class Node;
class Container;
class ResourceManager;
class Application;

int main_working_time = 0;

//Data Class
class Data {
	friend class File;
	friend class NameNode;
	private:
	int file_info;
        int data_info;
        int node_position;           //what node has this data
        bool is_cached;

	public:
	Data();
	Data(int nodenum, int fileidx, int dataidx);
	int GetFileInfo();
	int GetDataInfo();
	int GetNodePosition();
	bool IsCached();
};

//File Class
class File {
	friend class NameNode;
	private:
	int file_idx;
	int file_size;
	Data **datas;

	public:
	File(int fileidx, int size, int nodesize);
	int GetFileIdx();
	int GetFileSize();
	int GetNodePosition(int dataidx, int order);
	bool IsInNode(int nodenum, int dataidx);
	bool IsCached(int nodenum, int dataidx);
	bool IsCached(int dataidx);	
};

//NameNode Class
class NameNode {
	private:
	int node_size;
	int file_num;
	bool *failed_worker_check;
	int **node_file_num;
	
	public:
	File **file;
	NameNode(int nodesize, Node *worker[], int file_num, File *f[]);
	int GetNodeSize();
	int GetNodeFileNum(int nodeindex, int fileindex);
	int FindNode(int fileindex, int blockindex);
	int FindNode(int fileindex, int blockindex, int order);
	int *FindData(int nodeindex, int fileindex);
	bool IsInNode(int nodeindex, int fileindex, int blockindex);
};

//Container Class
class Container {
	friend class Node;
	friend class ResourceManager;
	private:
	int working_time;
	Application *task;
	int task_index;
	bool is_working;
	
	public:
	Container();
	void TaskExecute(Node *node, Application *job, int fileidx, int blockidx, string state); 
	int GetWorkingTime();
	bool GetIsWorking();
	int GetAvgTaskTime();
};

//Node Class
class Node {
	friend class ResourceManager;
	private:
	int node_idx;
	bool is_fail;
	int cache_size;
	int container_size;
	Container *container;

	public:
	Node(int idx, int cachesize, int contsize);
	int GetContSize();
};

//Application Class
class Application {
	friend class ResourceManager;
	private:
	int app_idx;		//app index
	string app_type;	//application name
	int file_idx;		//which file is needed
	int mapper_num;		
	int reducer_num;
	int avg_task_time_per_block;
	int skip_count;		
	int skip_checker;	//threshold
	bool *working_state;
	int task_pointer;
	int task_counter;	//threshold
	int access_time;
	
	public:
	Application(int appnum, string type, File *f, int rednum, int avgtasktime, int accesstime, int skipcount);
	void SetProcessed(int taskidx);
	void SetWorkingState(int taskidx);
	int GetAvgTaskTime();
	int GetSkipCount();
	bool IsCompleted();
	bool GetWorkingState(int taskidx);
	bool GetProcessingState(int taskidx);
	void AddTaskCounter();
};

class Grep : public Application {
        private:
                        
        public:         
        Grep(int appnum, string type, File *f, int rednum, int avgtasktime, int accesstime, int skipcount) : Application(appnum, type, f, rednum, avgtasktime, accesstime, skipcount) {
	}
};                      
class WordCount: public Application {
        private:
                        
        public:         
        WordCount(int appnum, string type, File *f, int rednum, int avgtasktime, int accesstime, int skipcount) : Application(appnum, type, f, rednum, avgtasktime, accesstime, skipcount) {
	}
};

//ResourceManager Class
class ResourceManager {
	private:
	Application *job_pointer;
	public:
	void JobCompleteManager(NameNode *NM, Node *nodes[]);
	void DelayScheduling(NameNode *NM, Node *nodes[], Application *jobs[], int app_num);
};

//Data Class implementation------------------------------------------------------------------------

Data::Data() {
	this->file_info = -1;
        this->data_info = -1;
        this->node_position = -1;
        this->is_cached = false;
}

Data::Data(int nodenum, int fileidx, int dataidx) {
	file_info = fileidx;
	data_info = dataidx;
	node_position = nodenum;
	is_cached = false;
}

int Data::GetFileInfo() { return file_info; }

int Data::GetDataInfo() { return data_info; }

int Data::GetNodePosition() { return node_position; }

bool Data::IsCached() { return is_cached; }

//File Class implementation------------------------------------------------------------------------

File::File(int fileidx, int size, int nodesize) {
	file_idx = fileidx;
	file_size = size;
	datas = new Data*[file_size];
	for(int i=0; i<file_size; i++) {
		datas[i] = new Data[3];
		for(int j=0; j<3; j++) {
			datas[i][j].file_info = file_idx;
			datas[i][j].data_info = i;
			datas[i][j].node_position = rand()%nodesize;
		}
	}
}

int File::GetFileIdx() { return file_idx; }

int File::GetFileSize() { return file_size; }

int File::GetNodePosition(int dataidx, int order) { return this->datas[dataidx][order].GetNodePosition(); }

bool File::IsInNode(int nodenum, int dataidx) {
	for(int i=0; i<3; i++) {
		if(nodenum == datas[dataidx][i].GetNodePosition()) {
			return true;
		}
	}
	return false;
}

bool File::IsCached(int nodenum, int dataidx) {
	for(int i=0; i<3; i++) {
		if(this->IsInNode(nodenum, dataidx)) {
			if(datas[dataidx][i].IsCached()) {
				return true;
			}
		}
	}
	return false;
}

bool File::IsCached(int dataidx) {
	for(int i=0; i<3; i++) { if(datas[dataidx][i].IsCached() == true) { return true; } }
	return false;
}

//NameNode Class implementation--------------------------------------------------------------------

NameNode::NameNode(int nodesize, Node *worker[], int filenum, File *f[]) {
	node_size = nodesize;
	file_num = filenum;
	cout << node_size;
	file = f;
	failed_worker_check = new bool[node_size];
	for(int i=0; i<node_size; i++) {
		failed_worker_check = false;
	}
	node_file_num = new int*[node_size];
	for(int i=0; i<node_size; i++) {
		node_file_num[i] = new int[file_num];
	}
}

int NameNode::GetNodeSize() { return node_size; }

int NameNode::GetNodeFileNum(int nodeindex, int fileindex) { return node_file_num[nodeindex][fileindex]; }

int NameNode::FindNode(int fileindex, int blockindex) {
        return this->file[fileindex]->GetNodePosition(blockindex, 0);
}

int NameNode::FindNode(int fileindex, int blockindex, int order) {
	return this->file[fileindex]->GetNodePosition(blockindex, order);	
}

int *NameNode::FindData(int nodeindex, int fileindex) {
	int arr_size_tmp = file[fileindex]->GetFileSize();
	int return_arr[arr_size_tmp];
	for(int i=0; i<arr_size_tmp; i++) {
		return_arr[i] = -1;
	}
	int flag = 0;
	for(int i=0; i<arr_size_tmp; i++) {
		for(int j=0; j<3; j++) {
			if(nodeindex == file[fileindex]->GetNodePosition(i, j)) {
				return_arr[flag] = i;
				flag++;
			}
		}
	}
	node_file_num[nodeindex][fileindex] = flag;
	int *return_arr_final = new int[flag];
	for(int i=0; i<flag; i++) {
		return_arr_final[i] = return_arr[i];
	}
	return return_arr_final;
}

bool NameNode::IsInNode(int nodeindex, int fileindex, int blockindex) {
	

}

//Container Class implementation-------------------------------------------------------------------

Container::Container() {
	is_working = false;
	working_time = 0;
}

void Container::TaskExecute(Node *node, Application *job, int fileidx, int blockidx, string state) {
	task = job;
	//cache local--------------------------------
	if(state == "cache_local") {
		working_time += job->GetAvgTaskTime();
		job->SetWorkingState(blockidx);
	}
}

int Container::GetWorkingTime() { return working_time; }

bool Container::GetIsWorking() { return is_working; }

int Container::GetAvgTaskTime() { return task->GetAvgTaskTime(); }

//Node Class implementation------------------------------------------------------------------------

Node::Node(int idx, int cachesize, int contsize) {
	node_idx = idx;
	is_fail = false;
	cache_size = cachesize;
	container_size = contsize;
	container = new Container[container_size];
}

int Node::GetContSize() { return container_size; }

//Application Class implementation--------------------------------------------------------------------

Application::Application(int appnum, string type, File *f, int rednum, int avgtasktime, int accesstime, int skipcount) {
	app_idx = appnum;
	app_type = type;
	file_idx = f->GetFileIdx();
	mapper_num = f->GetFileSize();
	reducer_num = rednum;
	task_counter = 0;
	task_pointer = 0;
	avg_task_time_per_block = avgtasktime;
	skip_count = skipcount;
	skip_checker = 0;
	working_state = new bool[mapper_num];
	access_time = accesstime;
	for(int i=0; i<mapper_num; i++) {
		working_state[i] = false;
	}
}

void Application::SetWorkingState(int taskidx) { working_state[taskidx] = true; }

int Application::GetAvgTaskTime() { return avg_task_time_per_block; }

int Application::GetSkipCount() {return skip_count; }

bool Application::IsCompleted() {
	if(skip_count == skip_checker) { return true; }
	else { return false; }
}

bool Application::GetWorkingState(int taskidx) { return working_state[taskidx]; }

void Application::AddTaskCounter() { task_counter++; }

//Resourece Manager implemenatation----------------------------------------------------------------

void ResourceManager::JobCompleteManager(NameNode *NM, Node *nodes[]) {
	for(int i=0; i<NM->GetNodeSize(); i++) {
		for(int j=0; j<nodes[0]->GetContSize(); j++) {
			if(nodes[i]->container[j].GetIsWorking() && nodes[i]->container[j].GetWorkingTime()+nodes[i]->container[j].GetAvgTaskTime() >= main_working_time) {
				nodes[i]->container[j].task->AddTaskCounter();
				nodes[i]->container[j].is_working = false;
			}
		}
	}
}

void ResourceManager::DelayScheduling(NameNode *NM, Node *nodes[], Application *jobs[], int app_num) {
	job_pointer = jobs[0];
	for(int i=0; i<NM->GetNodeSize(); i++) {
		for(int j=0; j<nodes[0]->GetContSize(); j++) {
			while(nodes[i]->container[j].GetIsWorking() == false) {
				if(job_pointer->IsCompleted()==false && NM->IsInNode(i , job_pointer->file_idx, job_pointer->task_pointer)==true) {
					nodes[i]->container[j].TaskExecute(nodes[i], job_pointer, job_pointer->file_idx, job_pointer->task_pointer);
				}
				if(job_pointer.IsCompleted()==false && NM.IsInNode(i, job_pointer->file_idx, job_pointer->task_pointer)==false) {
					job_pointer->skip_count++;
					if(job_pointer->skip_count >= job_pointer->skip_checker) {
						nodes[i]->container[j].TaskExecute(nodes[i], job_pointer, job_pointer->file_idx, job_pointer->task_pointer);
					}
				}
				job_pointer = job_pointer+4;
				//cycling in range
			}
		}
	}	
	
}

//Main Function
int main() {
	// Randomize
        srand((unsigned int)time(NULL));
	// 1. Node setting
        // First, we set each node's size, cache size, container number. Every node have same size.
        int node_num, cache_size, container_size;
        cout << "Set node numbers, cache size, number of container. \n";
        cin >> node_num >> cache_size >> container_size;

	Node *nodes[node_num];
        for(int i=0; i<node_num; i++) { nodes[i] = new Node(i, cache_size, container_size); }

        // 2. Data distribution into node
        int file_num;
        cout  << "How many files do you want to save? \n";
        cin >> file_num;
        File *files[file_num];

        int file_size;
        cout << "Set file size n times\n";
        for(int i=0; i<file_num; i++) {
                cin >> file_size;
                files[i] = new File(i, file_size, node_num);
                cout << "The file which size is " << file_size << " is stored and distributed in " << node_num << "nodes.\n";
        }
	NameNode Name(node_num, nodes, file_num, files);
        cout << "All files are stored!\n";
	cout << Name.GetNodeSize() << endl;

        for(int i=0; i<file_num; i++) {
		cout << "############"<<endl;
                for(int j=0; j<files[i]->GetFileSize(); j++) {
                        cout << files[i]->GetNodePosition(j, 0) << " ";
                }
		cout << endl;
                for(int j=0; j<files[i]->GetFileSize(); j++) {
                        cout << files[i]->GetNodePosition(j, 1) << " ";
                }
		cout << endl;
                for(int j=0; j<files[i]->GetFileSize(); j++) {
                        cout << files[i]->GetNodePosition(j, 2) << " ";
                }
                cout << endl;
        }

	cout << "Each node has these datas.\n";

	for(int i=0; i<node_num; i++) {
		cout << "Node " << i << " has\n";
		for(int j=0; j<file_num; j++) {
			int *arr = Name.FindData(i, j);
			for(int k=0; k<Name.GetNodeFileNum(i, j); k++) {
				cout << "(" << j << ", " << arr[k] << ") ";
			}
		}
		cout << endl;
	}

	// 3. Job Setting

	int app_num;
	string apptype;
	int inputfile;
	int reducenum;
	int avgtasktime;
	int requesttime;
	int skipcount;
	cout << "How many Apps do you assign?\n";
	cin >> app_num;
	Application *Apps[app_num];
	cout << "Set job type, input file, reduce tasks, average task time, requesting time, skip count.\n";
	for(int i=0; i<app_num; i++) {
		cin >> apptype >> inputfile >> reducenum >> avgtasktime >> requesttime >> skipcount;
		Apps[i] = new Application(i, apptype, files[inputfile], reducenum, avgtasktime, requesttime, skipcount);
	}
	cout << "Application setting is complete.\n";
	// 4. Main Task Start

//	while(true) {
		
		
//	}






	return 0;
}

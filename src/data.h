#include <stdio.h>
#include <iostream>
#include <map>
#include <queue>
#include <list>
#include <vector>
#include <string>
#include <fstream>
#include <time.h>
#include <Windows.h>


#define DEBUG
#define __UNSAFE__
#define NO_PROCESS_IS_RUNNING 0x7fffffff

//进程资源获取状态
#define OBTAINED_ALL_RESOURCE 1
#define SYSTEM_RESOURCE_NOT_ENOUGH 0
#define RESOURCE_NOT_ENOUGH_TEMP 2

//资源数目
#define n0 10
#define n1 10
#define n2 10
#define n3 10


using namespace std;

//process status
enum p_st{ ready = 0, running, blocked };
string getProcessStatusString(p_st st);

//resource type
enum ResourceType{ R0 = 0, R1, R2, R3 };
string getResourceTypeName(ResourceType ty);
ResourceType getTypeByStr(string str);

//资源
typedef struct {
	string r_id;    //resource id
	bool r_st;            //resource status,0 free,1 used
	ResourceType type;
	//可以定义资源动作
}resource;

////请求的资源数目和实例
typedef struct{
	int num;
	ResourceType type;  //加这个属性的目的是释放资源时要通过该属性来从资源仓库中查找到对应的RCB修改剩余数目
	list<resource * > occupied;
}need;

//进程控制块
class PCB{
public:
	PCB(PCB *Parent,string nm, int Prio, map<ResourceType, need *> need) :
		parent(Parent), name(nm),priority(Prio), other_resource(need){
		p_status = p_st::ready;
	}
	PCB(){}

	//因为懒写setter和getter所以就public了
public:
	unsigned int process_id;
	string name;
	map<ResourceType, need *> other_resource;  //进程需要的资源类型及数目
	bool getAllResource=0;
	p_st p_status;    //进程状态
	int queue_now;   //当前所在的队列,0,1,2为就绪队列，3为阻塞队列,4表示不在任何队列，running状态
	PCB *parent;    //进程树中的父节点
	PCB *child;    //进程树中的子节点
	int priority;    //进程优先级
};



//资源控制块
class RCB{
public:
	RCB(ResourceType type, int n) :ty(type), sum(n){
		this->remain = n;
		resource *r = new resource[n];
		for (int i = 0; i < n; i++){
			r[i].r_id = getResourceTypeName(type) + to_string(i);
			r[i].r_st = 0;
			r[i].type = type;
			res.push_back(r[i]);
		}
		delete[] r;
	}
	RCB(){}
public:
	ResourceType ty;     //resource type
	int sum;     //sum of resource
	int remain;   //remainder resource
	vector<resource> res;
	queue<int> w_queue;  //resource waiting queue
};
 
//资源仓库
static map<ResourceType, RCB*> storage;

//进程池
static map<int, PCB*> process_pool;

//就绪队列
static queue<int> ready_queue0;
static queue<int> ready_queue1;
static queue<int> ready_queue2;
//阻塞队列
static map<int,int> blocked_queue;
//运行进程
static int running_process = NO_PROCESS_IS_RUNNING;
//日志文件
static fstream f;


//注册资源RCB
void register_resource(ResourceType type,RCB *r);
//定义资源
void defineResource();
//获取资源RCB
RCB *getRCBbyType(ResourceType type);
//从进程池获取进程PCB
PCB *getPCBbyId(int id);
////从进程池中删除PCB
//void deletePCB(int id);
//获取不重复最小id
int getValidResourceId();
//写日志
void LOG(string str);



//资源申请
int request(int process_id);
//释放资源
void release(int process_id);
//创建进程
PCB *create(PCB *Parent, int Prio, map<ResourceType, need *> Need, string pn);
//进程调度
void scheduler();
//进程撤销
void destroy( int process_id);

void run(PCB *pcb);

void init();

//定时器中断
void timeOut();


//驱动程序
void showMenu();
void testShell();

void showAllResource();

void showAllProcess();
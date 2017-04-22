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

//������Դ��ȡ״̬
#define OBTAINED_ALL_RESOURCE 1
#define SYSTEM_RESOURCE_NOT_ENOUGH 0
#define RESOURCE_NOT_ENOUGH_TEMP 2

//��Դ��Ŀ
#define n0 10
#define n1 10
#define n2 10
#define n3 10


using namespace std;

//process status
enum p_st{ ready = 0, running, blocked };
string getProcessStatusString(p_st st);

//resource type
enum r_ty{ R0 = 0, R1, R2, R3 };
string getResourceTypeName(r_ty ty);
r_ty getTypeByStr(string str);

//��Դ
typedef struct {
	string r_id;    //resource id
	bool r_st;            //resource status,0 free,1 used
	r_ty type;
	//���Զ�����Դ����
}resource;

////�������Դ��Ŀ��ʵ��
typedef struct{
	int num;
	r_ty type;  //��������Ե�Ŀ�����ͷ���ԴʱҪͨ��������������Դ�ֿ��в��ҵ���Ӧ��RCB�޸�ʣ����Ŀ
	list<resource * > occupied;
}need;

//���̿��ƿ�
class PCB{
public:
	PCB(PCB *Parent,string nm, int Prio, map<r_ty, need *> need) :
		parent(Parent), name(nm),priority(Prio), other_resource(need){
		p_status = p_st::ready;
	}
	PCB(){}

	//��Ϊ��дsetter��getter���Ծ�public��
public:
	unsigned int process_id;
	string name;
	map<r_ty, need *> other_resource;  //������Ҫ����Դ���ͼ���Ŀ
	bool getAllResource=0;
	p_st p_status;    //����״̬
	int queue_now;   //��ǰ���ڵĶ���,0,1,2Ϊ�������У�3Ϊ��������,4��ʾ�����κζ��У�running״̬
	PCB *parent;    //�������еĸ��ڵ�
	PCB *child;    //�������е��ӽڵ�
	int priority;    //�������ȼ�
};



//��Դ���ƿ�
class RCB{
public:
	RCB(r_ty type, int n) :ty(type), sum(n){
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
	r_ty ty;     //resource type
	int sum;     //sum of resource
	int remain;   //remainder resource
	vector<resource> res;
	queue<int> w_queue;  //resource waiting queue
};
 
//��Դ�ֿ�
static map<r_ty, RCB*> storage;

//���̳�
static map<int, PCB*> process_pool;

//��������
static queue<int> ready_queue0;
static queue<int> ready_queue1;
static queue<int> ready_queue2;
//��������
static map<int,int> blocked_queue;
//���н���
static int running_process = NO_PROCESS_IS_RUNNING;
//��־�ļ�
static fstream f;


//ע����ԴRCB
void register_resource(r_ty type,RCB *r);
//������Դ
void defineResource();
//��ȡ��ԴRCB
RCB *getRCBbyType(r_ty type);
//�ӽ��̳ػ�ȡ����PCB
PCB *getPCBbyId(int id);
////�ӽ��̳���ɾ��PCB
//void deletePCB(int id);
//��ȡ���ظ���Сid
int getValidResourceId();
//д��־
void LOG(string str);



//��Դ����
int request(int process_id);
//�ͷ���Դ
void release(int process_id);
//��������
PCB *create(PCB *Parent, int Prio, map<r_ty, need *> Need, string pn);
//���̵���
void scheduler();
//���̳���
void destroy( int process_id);

void run(PCB *pcb);

void init();

//��ʱ���ж�
void timeOut();


//��������
void showMenu();
void testShell();

void showAllResource();

void showAllProcess();
#include"data.h"

string getProcessStatusString(p_st st){
	switch (st){
	case 0:return "ready";break;
	case 1:return "running";break;
	case 2:return "blocked";break;
	default:return "NULL";break;
	}
}

string getResourceTypeName(r_ty ty){
	switch (ty){
	case R0:return "r0";break;
	case R1:return "r1";break;
	case R2:return "r2";break;
	case R3:return "r3";break;
	default:return "NULL";break;
	}
}

r_ty getTypeByStr(string str){
	if (str == "R0")
		return r_ty::R0;
	else if (str == "R1")
		return r_ty::R1;
	else if (str == "R2")
		return r_ty::R2;
	else if (str == "R3")
		return r_ty::R3;
}

//注册资源RCB
void register_resource(r_ty type, RCB *r){
	storage[type] = r;
}
//定义资源
void defineResource(){
	//定义资源
	RCB *r0 = new RCB(r_ty::R0, n0);
	RCB *r1 = new RCB(r_ty::R1, n1);
	RCB *r2 = new RCB(r_ty::R2, n2);
	RCB *r3 = new RCB(r_ty::R3, n3);
	//for (int i = 0; i < 3; i++){
	//	cout << r0->res[i].r_id <<"   "<< r0->res[i].type<<endl;
	//} 
	register_resource(r_ty::R0, r0);
	register_resource(r_ty::R1, r1);
	register_resource(r_ty::R2, r2);
	register_resource(r_ty::R3, r3);
}
//获取资源RCB
RCB *getRCBbyType(r_ty type){
	auto it = storage.find(type);
	return it->second;
}

//获取进程PCB
PCB *getPCBbyId(int id){
	auto it = process_pool.find(id);
	if (it != process_pool.end())
		return it->second;
	else
		return nullptr;
}




//获取不重复最小id
int getValidId(){
	for (int i = 0; 1; i++){
		if (process_pool.find(i) == process_pool.end())
			return i;
	}
}
//写日志
void LOG(string str){
	f.open("LOG.txt", ios::out | ios::ate | ios::app);
	time_t t = time(NULL);
	struct tm* Time = localtime(&t);
	//mark 重定向到文件之后写文件不完整
	cout<< Time->tm_mon + 1 << "-"
		<< Time->tm_mday << " "
		<< Time->tm_hour << ":"
		<< Time->tm_min << ":"
		<< Time->tm_sec <<"  "<< str << endl;
}

void run(PCB *it){

	it->p_status = p_st::running;
	running_process = it->process_id;
	//LOG("LOG: @run," + it->name + " is running...");
	//mark 这里需不需要调度
	//scheduler();
}

void init(){
	map<r_ty, need *> rr;
	create(nullptr, 0, rr, "init");
	PCB *init = getPCBbyId(0);
	init->p_status = p_st::running;
	LOG("LOG: @init,init is running...");
	init->getAllResource = 1;
	running_process = init->process_id;
}

//定时器中断
void timeOut(){
		PCB *it = getPCBbyId(running_process);
		if (it == nullptr){
			//运行中的进程未找到，这不太可能发生，因为至少还有一个init虚拟进程
			LOG("LOG:  @time_out, PCB not found when try to found running_process,it may happened when the ready queue is empty.");
			return;
			//mark  warning，可以不处理
		}
		else{
			switch (it->queue_now){
			case 0:
				if (!ready_queue0.empty()&&ready_queue0.front()==it->process_id)
				ready_queue0.pop(); break;
			case 1:
				if (!ready_queue1.empty() && ready_queue1.front() == it->process_id)
				ready_queue1.pop(); break;
			case 2:
				if (!ready_queue2.empty() && ready_queue2.front() == it->process_id)
				ready_queue2.pop(); break;
			}
			it->p_status = p_st::ready;
			if (it->queue_now == 0);
			else if (it->queue_now > 0){
				it->queue_now--;
			}
			else{
				LOG("LOG: @timeOut，fatal error:queue_now < 0");
			}
		}

		//将中断的进程id入优先级对应的就绪队列
		LOG("LOG: @timeOut," + it->name + " timeout, enter ready queue " + to_string(it->queue_now) + " ,priority is " + to_string(it->priority));
		switch (it->queue_now){
		case 0:
			ready_queue0.push(it->process_id); break;
		case 1:
			ready_queue1.push(it->process_id); break;
		case 2:
			ready_queue2.push(it->process_id); break;
		}

		//唤醒最高优先级的进程，循环查找三个就绪队列，直到找到非空进程为止，并为之申请资源
		bool get2 = false, get1 = false;
		PCB *pcb = nullptr;
		//遍历ready_queue2
		while (1){
			if (ready_queue2.empty())
				break;
			else{
				if (pcb = getPCBbyId(ready_queue2.front())){
					if (request(pcb->process_id) == OBTAINED_ALL_RESOURCE){
						get2 = true;
						break;
					}
					else if (request(pcb->process_id) == SYSTEM_RESOURCE_NOT_ENOUGH){
						//mark 当系统资源不足时，这里处理完了没有？
						//销毁pcb，出就绪队列2，获取下一个进程。
						//destroy(pcb->process_id)-----已经在request里面销毁了
						LOG("LOG: @timeOut,when erdogic reasdy_queue2,one process has been destroied because system resource can not satisfy it!");
						//ready_queue2.pop();
						continue;
					}
					else if (request(pcb->process_id) == RESOURCE_NOT_ENOUGH_TEMP){
						LOG("LOG: @timeOut，" + pcb->name + "been blocked when request resource!");
						continue;
					}
				}
				//获取到的pcb为空的话
				else{
					ready_queue2.pop();
					continue;
				}
			}
		}

		//遍历ready_queue1
		while (1){
			if (get2 || ready_queue1.empty()) break;
			else if (pcb = getPCBbyId(ready_queue1.front())){
				if (request(pcb->process_id) == OBTAINED_ALL_RESOURCE){
					get1 = true;
					break;
				}
				else if (request(pcb->process_id) == SYSTEM_RESOURCE_NOT_ENOUGH){
					//mark 当系统资源不足时，这里处理完了没有？
					//销毁pcb，出就绪队列1，获取下一个进程。
					//destroy(pcb->process_id)-----已经在request里面销毁了
					LOG("LOG: @timeOut,when erdogic ready_queue1,one process has been destroied because system resource can not satisfy it!");
					//ready_queue1.pop();
					continue;
				}
				else if (request(pcb->process_id) == RESOURCE_NOT_ENOUGH_TEMP){
					LOG("LOG: @timeOut，" + pcb->name + "been blocked when request resource!");
					continue;
				}
			}
			//获取pcb为空时
			else{
				ready_queue1.pop();
				continue;
			}
		}

		//遍历ready_queue0
		while (1){

			if (get2 || get1) 
				break;
			else if (pcb = getPCBbyId(ready_queue0.front())){
				if (request(pcb->process_id) == OBTAINED_ALL_RESOURCE){
					break;
				}
				else if (request(pcb->process_id) == SYSTEM_RESOURCE_NOT_ENOUGH){
					//mark 当系统资源不足时，这里处理完了没有？
					//销毁pcb，出就绪队列0，获取下一个进程。
					//destroy(pcb->process_id)-----已经在request里面销毁了
					LOG("LOG: @timeOut,when erdogic reasdy_queue0,one process has been destroied because system resource can not satisfy it!");
					//ready_queue0.pop();
					continue;
				}
				else if (request(pcb->process_id) == RESOURCE_NOT_ENOUGH_TEMP){
					LOG("LOG: @timeOut，" + pcb->name + "been blocked when request resource!");
					continue;
				}
			}
			else{
				//因为init不会被阻塞，所以这种情况不可能出现）
				LOG("LOG: @timeOut,all ready queue is empty,timeOut return.");
				return;
			}
		}
		//运行
		run(pcb);
		LOG("LOG: @timeOut," + pcb->name + " is running......");
		scheduler();
}

//资源申请
int request(int process_id){   
	//所需资源数
	PCB *it= getPCBbyId(process_id);
	if (it == nullptr){
		LOG("LOG: @request，PCB not found ,when try to found process which is requesting resources,the process may have been destroied.");
	}
	//如果进程已经获取了所有资源的话
	else if (it->getAllResource)
		return OBTAINED_ALL_RESOURCE;
	else{
		map<r_ty, need *>::iterator iter;
		for (iter = it->other_resource.begin(); iter != it->other_resource.end(); iter++){
			int n = iter->second->num;
			RCB *rcb = getRCBbyType(iter->second->type);
			LOG("LOG: @request, " + it->name + " is requesting " + to_string(iter->second->num) + " resources "
				+ getResourceTypeName(rcb->ty));

			//如果进程所需资源数大于资源总数，释放已经获取的资源，销毁进程
			//出就绪队列
			if (n > rcb->sum){
				LOG("LOG: @request,system Resource Not Enough,when " + it->name + " request resource " + getResourceTypeName(rcb->ty));
				switch (it->queue_now){
				case 0:
					if (!ready_queue0.empty())
					ready_queue0.pop();
					break;
				case 1:
					if (!ready_queue1.empty())
					ready_queue1.pop();
					break;
				case 2:
					if (!ready_queue2.empty())
					ready_queue2.pop();
					break;
				}
				release(process_id);
				destroy(process_id);
				return SYSTEM_RESOURCE_NOT_ENOUGH;
			}

			//如果资源足够，则遍历该类资源仓库，修改n个资源的状态并将资源交给进程
			else if (n <= rcb->remain){
				rcb->remain -= n;
				//遍历各类资源线性表，找到n个空闲资源分配给进程
				int j = 0;
				//log
				string logStr = it->name + " get resource :";
				for (vector<resource>::iterator i = rcb->res.begin(); i != rcb->res.end(); i++){
					//若资源状态为非空闲
					if (i->r_st == 1){
						continue;
					}
					else{
						j++;
						i->r_st = 1;
						//将空闲资源分配给进程
						//mark bug
						iter->second->occupied.push_back(&(*i)); 
						//mark
						logStr += i->r_id + "     ";
					}
					if (j == n)
						break;
				}
				//mark 
				LOG("LOG: @request,"+logStr);
			}
			
			//如果资源不够，则阻塞进程
			else if (n > rcb->remain){
				//cout << "当资源不够时:"<< endl;
				release(it->process_id);  //释放已获取的资源
				it->p_status = p_st::blocked;   //修改进程状态为blocked
				LOG("LOG: @request," + it->name + " is blocked by " + getResourceTypeName(rcb->ty));
				switch (it->queue_now){
				case 0:
					if (!ready_queue0.empty())
					ready_queue0.pop();
					break;
				case 1:
					if (!ready_queue1.empty())
					ready_queue1.pop();
					break;
				case 2:
					if (!ready_queue2.empty())
					ready_queue2.pop();
					break;
				}
				//////mark error
				it->queue_now = 3;
				blocked_queue[it->process_id] = it->process_id;//入阻塞队列
				storage[iter->second->type]->w_queue.push(it->process_id);   //入该资源的等待队列
				scheduler();
				return RESOURCE_NOT_ENOUGH_TEMP;
			}
		}
		it->getAllResource = true;
	}
	return OBTAINED_ALL_RESOURCE;
}


//释放资源
void release(int process_id){
	PCB *pcb = getPCBbyId(process_id);
	if (pcb == nullptr){
		LOG("LOG: @release, PCB not found ,when try to found process which is releaseing resources,the process may have been destroied.");
	}
	map<r_ty, need *>::iterator it;
	for (it = pcb->other_resource.begin(); it != pcb->other_resource.end(); it++){
		//将进程中的资源返还，资源剩余数增加，资源状态修改为空闲
		RCB *rcb = getRCBbyType(it->first);
		rcb->remain += it->second->num;
		for (list<resource *>::iterator i = it->second->occupied.begin(); i != it->second->occupied.end(); i++){
			(*i)->r_st = 0;
		}
		//清空申请到的资源列表
		it->second->occupied.clear();

		//激活资源等待队列中队头的进程
		PCB *temp;
		while (!rcb->w_queue.empty()){
			temp = getPCBbyId(rcb->w_queue.front());
			if (temp == nullptr){
				LOG("LOG: @release, PCB not found ,when try to found the front process of blocked_queue,the process may have been destroied.");
				//cout << "the front process of ready_queue2 is not found!!!" << endl;
				rcb->w_queue.pop();
				continue;
			}
			else{
				temp->p_status = p_st::ready;
				temp->queue_now = temp->priority;
				//出阻塞队列
				//blocked_queue.erase[temp->process_id];
				LOG("LOG: @release, " + temp->name + " is activated by " + getResourceTypeName(it->first));
				switch (temp->queue_now){
				case 0:
					ready_queue0.push(temp->process_id); break;
				case 1:
					ready_queue1.push(temp->process_id); break;
				case 2:
					ready_queue2.push(temp->process_id); break;
				}
				//mark scheduler()
				scheduler();
				break;
			}
		}
	}
	//调用调度函数
	scheduler();
}


//创建进程
PCB *create(PCB *Parent, int Prio, map<r_ty, need *> Need, string pn){
	PCB *pcb = new PCB(Parent,pn, Prio, Need);
	pcb->queue_now = Prio;
	pcb->process_id = getValidId();
	LOG("LOG: @create,create process " + pcb->name + " with priority " + to_string(pcb->priority));
	//将进程添加到进程池
	process_pool[pcb->process_id] = pcb;
	//入就绪队列
	LOG("LOG: @create," + pcb->name + " enter ready_queue" + to_string(pcb->priority));
	switch (pcb->queue_now){
	case 0:
		ready_queue0.push(pcb->process_id); break;
	case 1:
		ready_queue1.push(pcb->process_id); break;
	case 2:
		ready_queue2.push(pcb->process_id); break;
	}
	//调用调度函数
	scheduler();
	return pcb;
}

//进程撤销
void destroy(int process_id){
	PCB *pcb=getPCBbyId(process_id);
	if (pcb == nullptr){
		LOG("LOG: @destroy, PCB not found!");
		return;
	}
	string logStr = "LOG: @destroy," + pcb->name + " is been destroied!";
	delete pcb;
	process_pool.erase(process_id);
	LOG(logStr);
}

//进程调度
void scheduler(){
	LOG("LOG: @scheduler,system is scheduling......");
	if (running_process != NO_PROCESS_IS_RUNNING){
		PCB *pcbRuning = getPCBbyId(running_process);
		if (pcbRuning == nullptr){
			LOG("LOG: @scheduler,PCB not found,when try to found the running process,the process may have been destroied.");
		}
		PCB *pcb;
		bool queue2_get = false;



		while (!ready_queue2.empty()){
			pcb = getPCBbyId(ready_queue2.front());
			if (pcb == nullptr){
				LOG("LOG: @scheduler,PCB not found,when try to found the front process of ready_queue2,the process may have been destroied.");
				ready_queue2.pop();
				continue;
			}
			else if (pcb->priority > pcbRuning->priority){
				pcbRuning->p_status = p_st::ready;
				switch (pcbRuning->queue_now){
				case 0:
					ready_queue0.push(running_process); break;
				case 1:
					ready_queue1.push(running_process); break;
				case 2:
					ready_queue2.push(running_process); break;
				}

				//marks_sheduler_resuest

				if (request(pcb->process_id) == OBTAINED_ALL_RESOURCE){
					run(pcb);
					LOG("LOG: @scheduler，" + pcb->name + " is running......");
					ready_queue2.pop();

					LOG("LOG: @scheduler, " + pcbRuning->name + " is grabed by " + pcb->name);
					//cout << pcb->name << " is running......" << endl;
					queue2_get = true;
					break;
				}
				else if (request(pcb->process_id) == SYSTEM_RESOURCE_NOT_ENOUGH){
					//mark 当系统资源不足时，这里处理完了没有？
					//销毁pcb，出就绪队列1，获取下一个进程。
					//destroy(pcb->process_id)-----已经在request里面销毁了
					LOG("LOG: @scheduler,when request resource for a activate process in ready_queue2,one process has been destroied because system resource can not satisfy it!");
					//ready_queue1.pop();
					continue;
				}
				else if (request(pcb->process_id) == RESOURCE_NOT_ENOUGH_TEMP){
					LOG("LOG: @scheduler，" + pcb->name + "been blocked when request resource!");
					continue;
				}
			}
			else break;
		}
		bool queue1_get = false;
		while (!queue2_get&&!ready_queue1.empty()) {
			PCB *pcb = getPCBbyId(ready_queue1.front());
			if (pcb == nullptr){
				LOG("LOG: @scheduler,PCB not found,when try to found the front process of ready_queue1,the process may have been destroied.");
				ready_queue1.pop();
				continue;
			}
			else if (pcb->priority > pcbRuning->priority){
				pcbRuning->p_status = p_st::ready;
				switch (pcbRuning->queue_now){
				case 0:
					ready_queue0.push(running_process); break;
				case 1:
					ready_queue1.push(running_process); break;
				case 2:
					ready_queue2.push(running_process); break;
				}
				//请求资源并运行
				if (request(pcb->process_id) == OBTAINED_ALL_RESOURCE){
					run(pcb);
					LOG("LOG: @scheduler，" + pcb->name + " is running......");
					ready_queue1.pop();
					LOG("LOG: @scheduler, " + pcbRuning->name + " is grabed by " + pcb->name);
					break;
				}
				else if (request(pcb->process_id) == SYSTEM_RESOURCE_NOT_ENOUGH){
					//mark 当系统资源不足时，这里处理完了没有？
					//销毁pcb，出就绪队列1，获取下一个进程。
					//destroy(pcb->process_id)-----已经在request里面销毁了
					LOG("LOG: @scheduler,when request resource for a activate process in ready_queue2,one process has been destroied because system resource can not satisfy it!");
					//ready_queue1.pop();
					continue;
				}
				else if (request(pcb->process_id) == RESOURCE_NOT_ENOUGH_TEMP){
					LOG("LOG: @scheduler，" + pcb->name + "been blocked when request resource!");
					continue;
				}
			}
			else break;
		}
	}
}

void showMenu(){
	cout << "\n-------------------------help-------------------------" << endl;
	cout << "please init and open shell file first." << endl;
	cout << " |-->help:                                           |" << endl;
	cout << " |      help/-help/--help/man                        |" << endl;
	cout << " |-->init task management procedure:                 |" << endl;
	cout << " |      init                                         |" << endl;
	cout << " |-->open shell file:                                |" << endl;
	cout << " |      [filename]                                   |" << endl;
	cout << " |-->create process:                                 |" << endl;
	cout << " |      cr [process name] [priority]                 |" << endl;
	cout << " |-->timeout running process:                        |" << endl;
	cout << " |      to                                           |" << endl;
	cout << " |-->show all resource or process:                   |" << endl;
	cout << " |      show [ar/ap]                                 |" << endl;
	cout << " |-->show real-time scheduler:                       |" << endl;
	cout << " |      top                                          |" << endl;
	cout << "------------------------------------------------------"<<endl;
	cout << endl;
}


//驱动程序
void testShell(){
	//string str = "cr P1 2 R0 2  R1 4 R2 2 R3 3";
	fstream file;
	string str;
	string temp;
	queue<string> strqueue;
	showMenu();

	while (1){
		//test.lxsh
		cout << "shell>>";
		cin >> str;

		//帮助
		if (str == "help" || str == "--help" || str == "-help"||str=="man"){
			showMenu();
			continue;
		}
		else if (str == "init"){
			init();
		}
		else if (str == "to"){
			timeOut();
			continue;
		}
		else if (str == "show ar"){
			//if (running_process == NO_PROCESS_IS_RUNNING){
			//	LOG("LOG: @testshell,no process is running,create some processes first");
			//	showMenu();
			//	continue;
			//}
			//else
				showAllResource();
		}
		else if (str == "show ap"){
			showAllProcess();
		}
		else if (str == "top"){
			while (1){
				timeOut();
				cout << endl;
				showAllResource();
				showAllProcess();
				Sleep(1000);
				system("cls");
			}
		}
		else{
			file.open(str+".txt");
			if (!file.is_open()){
				LOG("LOG: @testShell,undefined command!!1!");
				continue;
			}
			else{
				while (!file.eof()){
					getline(file, str);
					//cout << str << endl;
					//字符串解析
					if (str.size() != 0){
						int i = 0;
						int j = 0;
						str += ' ';
						while (str.find(" ", i) != string::npos){
							j = str.find(" ", i);
							temp = str.substr(i, j - i);
							strqueue.push(temp);
							i = j + 1;
						}
					}

					if (strqueue.front() == "cr"){
						strqueue.pop();
						string name = strqueue.front();
						strqueue.pop();
						int prio = atoi(strqueue.front().c_str());
						strqueue.pop();
						//cout << name << "   " << prio << endl;

						map<r_ty, need *> res;
						//int i = 0;
						while (!strqueue.empty()){
							need *ne = new need;
							ne->type = getTypeByStr(strqueue.front());
							strqueue.pop();
							//cout << i << endl;
							ne->num = atoi(strqueue.front().c_str());
							strqueue.pop();
							//cout << i++;
							res[ne->type] = ne;
						}
						create(getPCBbyId(0), prio, res, name);
					}
				}
			}
		}
	}
}



void showAllResource(){
	cout << "ALL RESOURCES--------------------------------------------------------------------------------"<<endl;
	for (map<r_ty, RCB*>::iterator it = storage.begin(); it != storage.end(); it++){
		cout << getResourceTypeName(it->first) << "|";
		for (vector<resource>::iterator res = it->second->res.begin(); res != it->second->res.end(); res++){
			cout << " "<< res->r_id << "(" << res->r_st << ") |";
		}
		cout << endl;
	}
	cout << "---------------------------------------------------------------------------------------------" << endl;
}

void showAllProcess(){
	cout << "ALL PROCESSES-----------------------------" <<endl;
	cout << "PID   " << "PNAME   " << "PRIORITY   " <<" queue_now "<< "STATUS|" << endl;
	for (map<int, PCB*>::iterator it = process_pool.begin(); it != process_pool.end(); it++){
		cout << " " << it->second->process_id 
			<< "    " << it->second->name
			<< "       " << it->second->priority 
			<< "            "<<it->second->queue_now
			<< "        " << it->second->p_status <<"  |"<< endl;
	}
	cout << "-----------------------------------------" << endl;
}
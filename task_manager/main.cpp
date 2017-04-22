//bug、优化、待完成部分
//√+进程调度低优先级进程存在别饿死情况
//       可采用多级反馈队列调度算法
//          1为每个队列增加时间片属性，优先级越低时间片越长
//          2时间片内未运行完成则进入低优先级，queue_now减1.（从2开始递减）
//          3使用queue_now标识进程所在队列，所以除初始值外，其他时刻它与priority不一定相同
//          4最低优先级不再降低，按照时间片轮转调度
//+timeOut 有bug
//√+日志文件加上函数位置
//√+资源请求数大于系统资源数时崩溃
//+test shell
//+从文件输入脚本
//+日志文件输出问题
//同时有两个进程状态为running
//从未出阻塞队列






#include "data.h"
using namespace std;



int main(int argc, char **argv){
	//加载资源
	defineResource();
	//创建init，默认init位于资源池的开始，即map<0,init>
	//init();
	testShell();

	system("pause");
	return 0;

}
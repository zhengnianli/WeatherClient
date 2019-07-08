# 【socket应用】基于C语言的天气客户端的实现



# 一、前言

上一篇笔记分享了[【socket笔记】TCP、UDP通信总结](https://zhengnianli.github.io/2019/06/30/socket-bi-ji-tcp-udp-tong-xin-zong-jie/)，这一篇分享一个用C语言写的、基于TCP的一个HTTP天气客户端的实现，这个一个控制台应用程序，最终的界面如下：

![ZDKRxA.png](https://s2.ax1x.com/2019/07/07/ZDKRxA.png)

关于天气预报，之前我已经用`STM32+ESP8266 `wifi模块实现过了一遍，感兴趣的可查阅往期笔记：[基于STM32的智能天气预报系统](https://zhengnianli.github.io/2019/06/20/zuo-pin-ji-yu-stm32-de-zhi-neng-tian-qi-yu-bao-xi-tong/)。这次这个基于C语言控制台程序的HTTP客户端的天气解析的代码和之前分享的差不多，只是在那基础上添加修改了一些东西，并配合socket的相关知识实现的，以巩固一下socket编程的知识。下面分享一些实现过程。



# 二、天气客户端实现的要点

首先，需要说明的是，这份代码是在windows系统下使用`gcc6.3.0`进行编译的。

## 1、秘钥

> 心知天气：www.seniverse.com

我们完成这个实验必须得到这个上面去注册一个账号才能使用它的天气数据，注册之后每个账户都会有一个`私钥`，例如：

> 私钥 SMEieQjde1C9eXnbE

这个是我们程序中需要用到。

## 2、IP和端口

上一节分享了socket的笔记，我们与服务端通信，需要知道三个重要的信息，分别是：

1. IP地址
2. 端口
3. 传输方式

这里的心知天气的IP是`116.62.81.138`，端口是`80`，传输方式是`TCP`，对应的代码如下：

```c
/* 设置要访问的服务器的信息 */
SOCKADDR_IN  ServerSockAddr;
memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));  		  // 每个字节都用0填充
ServerSockAddr.sin_family = PF_INET;						  // IPv4
ServerSockAddr.sin_addr.s_addr = inet_addr(WEATHER_IP_ADDR);  // 心知天气服务器IP
ServerSockAddr.sin_port = htons(WEATHER_PORT);   			  // 端口
```
这里的`WEATHER_IP_ADDR`对应的就是`116.62.81.138`，`WEATHER_PORT`对应的就是`80`。

## 3、GET请求

HTTP有几种请求方法，我们这里使用的是`GET`请求。查看心知天气API文档可知，请求地址示例为：

> https://api.seniverse.com/v3/weather/now.json?key=your_api_key&location=beijing&language=zh-Hans&unit=c

这是一个天气实况的请求地址示例，其有几个重要的参数：

![ZD8cPf.png](https://s2.ax1x.com/2019/07/07/ZD8cPf.png)

这里的`key`是个很重要的参数，就是我们前面说的`私钥`。

我们的天气客户端就是要往天气服务端发送类似这样的`GET请求`来获取天气数据，具体的请求方法示例为：

```c
GET https://api.seniverse.com/v3/weather/now.json?key=2owqvhhd2dd9o9f8&location=beijing&language=zh-Hans&unit=c
```

对应代码如下：

```c
/* 秘钥，注意！！如果要用这一份代码，这个一定要改为自己的，因为这个我已经故意改错了，防止有人与我公用一个KEY */
#define  KEY    "2owqvhhd2dd9o9f8"		// 这是在心知天气注册后，每个用户自己的一个key

/* GET请求包 */
#define  GET_REQUEST_PACKAGE     \
         "GET https://api.seniverse.com/v3/weather/%s.json?key=%s&location=%s&language=zh-Hans&unit=c\r\n\r\n"
	
/* JSON数据包 */	
#define  NOW_JSON     "now"
#define  DAILY_JSON   "daily"
//....还用更多其他的天气数据包可查阅心知天气

/* 组合GET请求包 */
sprintf(GetRequestBuf, GET_REQUEST_PACKAGE, weather_json, KEY, location);
	
/* 发送数据到服务端 */
send(ClientSock, GetRequestBuf, strlen(GetRequestBuf), 0);
```

这里简单复习一下`sprintf函数`的用法：

（1）函数功能：字符串格式化

（2）函数原型：int sprintf(char *string, char *format [,argument,...]);

**string**： 这是指向一个字符数组的指针，该数组存储了 C 字符串。

**format** ： 这是字符串，包含了要被写入到字符串 str 的文本。

***[argument]..*****.**：根据不同的 format 字符串，函数可能需要一系列的附加参数，每个参数包含了一个要被插入的值，替换了 format 参数中指定的每个 % 标签。

（3）使用示例：

```c
sprintf(buf, "%s,%d", str, num);
```

假如此时`str`为`"hello"`，`num`为`5201314`，则此时buf中的内容为：`hello，5201314`，需要注意的是buf的容量要足够大。

## 4、天气服务端返回的数据

天气服务端给我们天气客户端返回的数据为`JSON`格式数据，可查阅往期笔记[JSON的简单认识](https://zhengnianli.github.io/2019/06/15/json-de-jian-dan-ren-shi/)。我们这个天气客户端只是实现了查询此刻天气（对应的数据包为`now.json`）及近三天天气情况（对应的数据包为`daily.json`），如要查询其他信息，可模仿我们这里处理`now.json`和`daily.json`的方法，我们用`cJson库`进行解析。

这个`cJson`库的下载链接为：

> 链接：https://pan.baidu.com/s/1DQynsdlNyIvsVXmf4W5b8Q
> 提取码：ww4z

只要把`cJSON.c`与`cJSON.ｈ`放到工程主程序所在目录，然后在主程序中包含头文件`JSON.ｈ`即可引入该库。如：

![ZDYU2T.png](https://s2.ax1x.com/2019/07/07/ZDYU2T.png)

为了解析`now.json`和`daily.json`中的有用数据，我们建立如下结构体：

```c
/* 天气数据结构体 */
typedef struct
{
	/* 实况天气数据 */
	char id[32];				//id
	char name[32];				//地名
	char country[32];			//国家
	char path[32];				//完整地名路径
	char timezone[32];			//时区
	char timezone_offset[32];   //时差
	char text[32];				//天气预报文字
	char code[32];				//天气预报代码
	char temperature[32];   	//气温
	char last_update[32];		//最后一次更新的时间
	
	
	/* 今天、明天、后天天气数据 */
	char date[3][32];			//日期
	char text_day[3][64];	    //白天天气现象文字
	char code_day[3][32];		//白天天气现象代码
	char code_night[3][64]; 	//晚间天气现象代码
	char high[3][32];			//最高温
	char low[3][32];			//最低温
	char wind_direction[3][64]; //风向
	char wind_speed[3][32];  	//风速，单位km/h（当unit=c时）
	char wind_scale[3][32];  	//风力等级
}Weather;
```



现在看一下`now.json`和`daily.json`的内容是怎样的：

（1）`now.json`示例及解析：

**now.json：**

```json
{
  "results": [
    {
      "location": {
        "id": "C23NB62W20TF",
        "name": "西雅图",
        "country": "US",
        "path": "西雅图,华盛顿州,美国",
        "timezone": "America/Los_Angeles",
        "timezone_offset": "-07:00"
      },
      "now": {
        "text": "多云", //天气现象文字
        "code": "4", //天气现象代码
        "temperature": "14", //温度，单位为c摄氏度或f华氏度
        "feels_like": "14", //体感温度，单位为c摄氏度或f华氏度
        "pressure": "1018", //气压，单位为mb百帕或in英寸
        "humidity": "76", //相对湿度，0~100，单位为百分比
        "visibility": "16.09", //能见度，单位为km公里或mi英里
        "wind_direction": "西北", //风向文字
        "wind_direction_degree": "340", //风向角度，范围0~360，0为正北，90为正东，180为正南，270为正西
        "wind_speed": "8.05", //风速，单位为km/h公里每小时或mph英里每小时
        "wind_scale": "2", //风力等级，请参考：http://baike.baidu.com/view/465076.htm
        "clouds": "90", //云量，单位%，范围0~100，天空被云覆盖的百分比 #目前不支持中国城市#
        "dew_point": "-12" //露点温度，请参考：http://baike.baidu.com/view/118348.htm #目前不支持中国城市#
      },
      "last_update": "2015-09-25T22:45:00-07:00" //数据更新时间（该城市的本地时间）
    }
  ]
}
```

这里实测了一下，我们普通用户（因为没充钱，哈哈~）申请的`now.json`数据中，`now对象`中只有如下三个键值对：

```json
"text": "多云", //天气现象文字
"code": "4", //天气现象代码
"temperature": "14", //温度，单位为c摄氏度或f华氏度
```

**now.json的解析函数：**

```c
/*******************************************************************************************************
** 函数: cJSON_NowWeatherParse，解析天气实况数据
**------------------------------------------------------------------------------------------------------
** 参数: JSON：天气数据包   result：数据解析的结果
** 返回: void
********************************************************************************************************/
static int cJSON_NowWeatherParse(char *JSON, Weather *result)
{
	cJSON *json,*arrayItem,*object,*subobject,*item;
	
	json = cJSON_Parse(JSON); //解析JSON数据包
	if(json == NULL)		  //检测JSON数据包是否存在语法上的错误，返回NULL表示数据包无效
	{
		printf("Error before: [%s]\n",cJSON_GetErrorPtr()); //打印数据包语法错误的位置
		return 1;
	}
	else
	{
		if((arrayItem = cJSON_GetObjectItem(json,"results")) != NULL); //匹配字符串"results",获取数组内容
		{
			int size = cJSON_GetArraySize(arrayItem);     //获取数组中对象个数
#if DEBUG
			printf("cJSON_GetArraySize: size=%d\n",size); 
#endif
			if((object = cJSON_GetArrayItem(arrayItem,0)) != NULL)//获取父对象内容
			{
				/* 匹配子对象1：城市地区相关 */
				if((subobject = cJSON_GetObjectItem(object,"location")) != NULL)
				{
					// 匹配id
					if((item = cJSON_GetObjectItem(subobject,"id")) != NULL)   
					{
						memcpy(result->id, item->valuestring,strlen(item->valuestring)); 		// 保存数据供外部调用
					}
					// 匹配城市名
					if((item = cJSON_GetObjectItem(subobject,"name")) != NULL) 
					{
						memcpy(result->name, item->valuestring,strlen(item->valuestring)); 		// 保存数据供外部调用
					}
					// 匹配城市所在的国家
					if((item = cJSON_GetObjectItem(subobject,"country")) != NULL)
					{
						memcpy(result->country, item->valuestring,strlen(item->valuestring)); 	// 保存数据供外部调用
					}
					// 匹配完整地名路径
					if((item = cJSON_GetObjectItem(subobject,"path")) != NULL)  
					{
						memcpy(result->path, item->valuestring,strlen(item->valuestring)); 		// 保存数据供外部调用	
					}
					// 匹配时区
					if((item = cJSON_GetObjectItem(subobject,"timezone")) != NULL)
					{
						memcpy(result->timezone, item->valuestring,strlen(item->valuestring)); 	// 保存数据供外部调用	
					}
					// 匹配时差
					if((item = cJSON_GetObjectItem(subobject,"timezone_offset")) != NULL)
					{
						memcpy(result->timezone_offset, item->valuestring,strlen(item->valuestring)); 	// 保存数据供外部调用
					}
				}
				/* 匹配子对象2：今天的天气情况 */
				if((subobject = cJSON_GetObjectItem(object,"now")) != NULL)
				{
					// 匹配天气现象文字
					if((item = cJSON_GetObjectItem(subobject,"text")) != NULL)
					{
						memcpy(result->text, item->valuestring,strlen(item->valuestring));  // 保存数据供外部调用
					}
					// 匹配天气现象代码
					if((item = cJSON_GetObjectItem(subobject,"code")) != NULL)
					{
						memcpy(result->code, item->valuestring,strlen(item->valuestring));  // 保存数据供外部调用
					}
					// 匹配气温
					if((item = cJSON_GetObjectItem(subobject,"temperature")) != NULL) 
					{
						memcpy(result->temperature, item->valuestring,strlen(item->valuestring));   // 保存数据供外部调用
					}	
				}
				/* 匹配子对象3：数据更新时间（该城市的本地时间） */
				if((subobject = cJSON_GetObjectItem(object,"last_update")) != NULL)
				{
					memcpy(result->last_update, subobject->valuestring,strlen(subobject->valuestring));   // 保存数据供外部调用
				}
			} 
		}
	}
	
	cJSON_Delete(json); //释放cJSON_Parse()分配出来的内存空间
	
	return 0;
}
```



（2）`daily.json`示例及解析：

**daily.json：**

```json
{
  "results": [{
    "location": {
      "id": "WX4FBXXFKE4F",
      "name": "北京",
      "country": "CN",
      "path": "北京,北京,中国",
      "timezone": "Asia/Shanghai",
      "timezone_offset": "+08:00"
    },
    "daily": [{                         //返回指定days天数的结果
      "date": "2015-09-20",             //日期
      "text_day": "多云",               //白天天气现象文字
      "code_day": "4",                  //白天天气现象代码
      "text_night": "晴",               //晚间天气现象文字
      "code_night": "0",                //晚间天气现象代码
      "high": "26",                     //当天最高温度
      "low": "17",                      //当天最低温度
      "precip": "0",                    //降水概率，范围0~100，单位百分比（目前仅支持国外城市）
      "wind_direction": "",             //风向文字
      "wind_direction_degree": "255",   //风向角度，范围0~360
      "wind_speed": "9.66",             //风速，单位km/h（当unit=c时）、mph（当unit=f时）
      "wind_scale": ""                  //风力等级
    }, {
      "date": "2015-09-21",
      "text_day": "晴",
      "code_day": "0",
      "text_night": "晴",
      "code_night": "0",
      "high": "27",
      "low": "17",
      "precip": "0",
      "wind_direction": "",
      "wind_direction_degree": "157",
      "wind_speed": "17.7",
      "wind_scale": "3"
    }, {
      ...                               //更多返回结果
    }],
    "last_update": "2015-09-20T18:00:00+08:00" //数据更新时间（该城市的本地时间）
  }]
}
```

**daily.json解析函数：**

```c
/*******************************************************************************************************
** 函数: cJSON_DailyWeatherParse，解析近三天天气数据
**------------------------------------------------------------------------------------------------------
** 参数: JSON：天气数据包   result：数据解析的结果
** 返回: void
********************************************************************************************************/
static int cJSON_DailyWeatherParse(char *JSON, Weather *result)
{
	cJSON *json,*arrayItem,*object,*subobject,*item,*sub_child_object,*child_Item;
	
	json = cJSON_Parse(JSON); //解析JSON数据包
	if(json == NULL)		  //检测JSON数据包是否存在语法上的错误，返回NULL表示数据包无效
	{
		printf("Error before: [%s]\n",cJSON_GetErrorPtr()); //打印数据包语法错误的位置
		return 1;
	}
	else
	{
		if((arrayItem = cJSON_GetObjectItem(json,"results")) != NULL); //匹配字符串"results",获取数组内容
		{
			int size = cJSON_GetArraySize(arrayItem);     //获取数组中对象个数
#if DEBUG
			printf("Get Array Size: size=%d\n",size); 
#endif
			if((object = cJSON_GetArrayItem(arrayItem,0)) != NULL)//获取父对象内容
			{
				/* 匹配子对象1------结构体location */
				if((subobject = cJSON_GetObjectItem(object,"location")) != NULL)
				{
					if((item = cJSON_GetObjectItem(subobject,"name")) != NULL) //匹配子对象1成员"name"
					{
						memcpy(result->name, item->valuestring,strlen(item->valuestring)); 		// 保存数据供外部调用
					}
				}
				/* 匹配子对象2------数组daily */
				if((subobject = cJSON_GetObjectItem(object,"daily")) != NULL)
				{
					int sub_array_size = cJSON_GetArraySize(subobject);
#if DEBUG
					printf("Get Sub Array Size: sub_array_size=%d\n",sub_array_size);
#endif
					for(int i = 0; i < sub_array_size; i++)
					{
						if((sub_child_object = cJSON_GetArrayItem(subobject,i))!=NULL)
						{
							// 匹配日期
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"date")) != NULL)
							{
								memcpy(result->date[i], child_Item->valuestring,strlen(child_Item->valuestring)); 		// 保存数据
							}
							// 匹配白天天气现象文字
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"text_day")) != NULL)
							{
								memcpy(result->text_day[i], child_Item->valuestring,strlen(child_Item->valuestring)); 	// 保存数据
							}
							// 匹配白天天气现象代码
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"code_day")) != NULL)
							{
								memcpy(result->code_day[i], child_Item->valuestring,strlen(child_Item->valuestring)); 	// 保存数据
							}
							// 匹配夜间天气现象代码
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"code_night")) != NULL)
							{
								memcpy(result->code_night[i], child_Item->valuestring,strlen(child_Item->valuestring)); // 保存数据
							}
							// 匹配最高温度
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"high")) != NULL)
							{
								memcpy(result->high[i], child_Item->valuestring,strlen(child_Item->valuestring)); 		//保存数据
							}
							// 匹配最低温度
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"low")) != NULL)
							{
								memcpy(result->low[i], child_Item->valuestring,strlen(child_Item->valuestring)); 		// 保存数据
							}
							// 匹配风向
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"wind_direction")) != NULL)
							{
								memcpy(result->wind_direction[i],child_Item->valuestring,strlen(child_Item->valuestring)); //保存数据
							}
							// 匹配风速，单位km/h（当unit=c时）
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"wind_speed")) != NULL)
							{
								memcpy(result->wind_speed[i], child_Item->valuestring,strlen(child_Item->valuestring)); // 保存数据
							}
							// 匹配风力等级
							if((child_Item = cJSON_GetObjectItem(sub_child_object,"wind_scale")) != NULL)
							{
								memcpy(result->wind_scale[i], child_Item->valuestring,strlen(child_Item->valuestring)); // 保存数据
							}
						}
					}
				}
				/* 匹配子对象3------最后一次更新的时间 */
				if((subobject = cJSON_GetObjectItem(object,"last_update")) != NULL)
				{
					//printf("%s:%s\n",subobject->string,subobject->valuestring);
				}
			} 
		}
	}
	
	cJSON_Delete(json); //释放cJSON_Parse()分配出来的内存空间
	
	return 0;
}
```

## 5、获取天气数据并解析

这个函数就涉及到我们上一节笔记中的socket编程的知识了，先看一下这个函数实现的总体框图：

![ZD2emR.png](https://s2.ax1x.com/2019/07/08/ZD2emR.png)

下面是函数实现的细节过程：

```c
/*******************************************************************************************************
** 函数: GetWeather，获取天气数据并解析
**------------------------------------------------------------------------------------------------------
** 参数: weather_json：需要解析的json包   location：地名   result：数据解析的结果
** 返回: void
********************************************************************************************************/
static void GetWeather(char *weather_json, char *location, Weather *result)
{
	SOCKET ClientSock;
	WSADATA wd;
	char GetRequestBuf[256] = {0};
	char WeatherRecvBuf[2*1024] = {0};
	char GbkRecvBuf[2*1024] = {0};
	int  gbk_recv_len = 0;
	int  connect_status = 0;
	
	/* 初始化操作sock需要的DLL */
	WSAStartup(MAKEWORD(2,2),&wd);  
	
	/* 设置要访问的服务器的信息 */
    SOCKADDR_IN  ServerSockAddr;
    memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));  		  // 每个字节都用0填充
    ServerSockAddr.sin_family = PF_INET;						  // IPv4
    ServerSockAddr.sin_addr.s_addr = inet_addr(WEATHER_IP_ADDR);  // 心知天气服务器IP
    ServerSockAddr.sin_port = htons(WEATHER_PORT);   			  // 端口
	
	/* 创建客户端socket */
	if (-1 == (ClientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)))
	{
		printf("socket error!\n");
		exit(1);
	}
	
	/* 连接服务端 */
	if (-1 == (connect_status = connect(ClientSock, (SOCKADDR*)&ServerSockAddr, sizeof(SOCKADDR))))
	{
		printf("connect error!\n");
		exit(1);
	}
	
	/* 组合GET请求包 */
	sprintf(GetRequestBuf, GET_REQUEST_PACKAGE, weather_json, KEY, location);
	
	/* 发送数据到服务端 */
	send(ClientSock, GetRequestBuf, strlen(GetRequestBuf), 0);
		
	/* 接受服务端的返回数据 */
	recv(ClientSock, WeatherRecvBuf, 2*1024, 0);
	
	/* utf-8转为gbk */
	SwitchToGbk((const unsigned char*)WeatherRecvBuf, strlen((const char*)WeatherRecvBuf), (unsigned char*)GbkRecvBuf, &gbk_recv_len);	
#if DEBUG
	printf("服务端返回的数据为：%s\n", GbkRecvBuf);
#endif
	
	/* 解析天气数据并保存到结构体变量weather_data中 */
	if (0 == strcmp(weather_json, NOW_JSON))		// 天气实况
	{
		cJSON_NowWeatherParse(GbkRecvBuf, result);	
	}
	else if(0 == strcmp(weather_json, DAILY_JSON)) // 未来三天天气
	{
		cJSON_DailyWeatherParse(GbkRecvBuf, result);	
	}
	
	/* 清空缓冲区 */
	memset(GetRequestBuf, 0, 256);   
	memset(WeatherRecvBuf, 0, 2*1024);   
	memset(GbkRecvBuf, 0, 2*1024); 
	
	/* 关闭套接字 */
	closesocket(ClientSock);  
	
	/* 终止使用 DLL */
	WSACleanup();  
}

```

## 6、编译

如何编译这份代码（可在文末进行获取）呢？

这份C代码工程的文件如下：

![ZDRFEt.png](https://s2.ax1x.com/2019/07/08/ZDRFEt.png)

在windows系统下使用`gcc`编译器编译，编译命令为：

```c
gcc weather_client.c cJSON.c utf8togbk.c -o weather_client.exe -lwsock32
```

如：

![ZDRVC8.png](https://s2.ax1x.com/2019/07/08/ZDRVC8.png)

这里的`weather_client.exe`就是我们编译生成的可执行文件：`天气客户端`，双击就可以运行了。此外，`-lwsock32`参数上一节也有讲过，这个参数用于链接`windows`下socket编程必须的`winsock2`这个库。若是使用集成开发环境，则需要把`wsock32.lib`放在工程目录下，并在我们代码中`#include <winsock2.h>` 下面加上一行 `#pragma comment(lib, "ws2_32.lib")`代码（在IDE里编译本人未验证，有兴趣的朋友可尝试）。



需要说明的是，Windows下默认是没有装gcc的，需要自己进行配置，关于配置及使用`mingw`（这是个工具包，里面包含有gcc编译器）可查看往期笔记：[【C语言笔记】使用notepad++、MinGW来开发C程序](https://zhengnianli.github.io/2018/10/05/c-yu-yan-bi-ji-shi-yong-notepad-mingw-lai-kai-fa-c-cheng-xu/)、[【C语言笔记】windows命令行下编译C程序](https://zhengnianli.github.io/2018/12/23/c-yu-yan-bi-ji-windows-ming-ling-xing-xia-bian-yi-c-cheng-xu/)

## 7、运行结果示例

![ZD4Vds.png](https://s2.ax1x.com/2019/07/08/ZD4Vds.png)

此处，只能使用拼音进行搜索，其实也可以做输入汉字进行搜索的功能，只是要进行转码处理，这个功能实现在[基于STM32的智能天气预报系统](https://zhengnianli.github.io/2019/06/20/zuo-pin-ji-yu-stm32-de-zhi-neng-tian-qi-yu-bao-xi-tong/)的代码里已经有做，有兴趣的朋友可以参考这个。

# 三、代码获取

> https://github.com/zhengnianli/WeatherClient


---

我的个人博客为：https://zhengnianli.github.io

我的微信公众号为：嵌入式大杂烩

[![VcSFJJ.md.png](https://s2.ax1x.com/2019/06/11/VcSFJJ.md.png)](https://imgchr.com/i/VcSFJJ)


#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <errno.h>
#include <pthread.h>


#define CS_MCP3208 6 //default : 6



#define SPI_CHANNEL 0	//ABS
#define SPI_CHANNEL4 4	//FSR1
#define SPI_CHANNEL6 6	//FSR2
#define SPI_CHANNEL7 7	//FSR3

#define SPI_SPEED 1000000   //1MHz
#define COUNT 10
#define SOUND 23
#define BUTTON 20
int flag = 0;//SEND FLAG
int night = 1;//RECEIVE TIME FLAG !!!In EXHIBITION SET 1!!!///
typedef struct 
{
	int bed_no;
	int abs_fsr;
	int fsr1;
	int fsr2;
	int fsr3;
	int noise;
	int button;
	int sock;
}Frame;
typedef struct
{
	int sec;
	int min;
	int hour;
	int day;
	int mon;
	int year;
	int wday;
	int yday;
	int isdt;
}Time;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
Frame Data;
int cnt = 1;
double AVG_0 = 0, AVG_4 = 0, AVG_6 = 0, AVG_7 = 0, AVG_23 = 0;
int event =0;
void* TX_pthread(void* args)
{
	int sock_desc = (int) args;
	
	
	for(;;)
	{
		AVG_0 = AVG_0 / cnt;  AVG_4 = AVG_4 / cnt; AVG_6 = AVG_6 / cnt;   AVG_7 = AVG_7 / cnt;  AVG_23 = AVG_23 / cnt;
		getchar();
		printf("[AVG_CH0 : %.2lf], [AVG_CH4 : %.2lf], [AVG_CH7 : %.2lf], [AVG_CH8 : %.2lf], [AVG_CH23 : %.2lf]\n", AVG_0, AVG_4, AVG_6, AVG_7, AVG_23);



		//////////////////////////////////////////////////////////////////////////////////
		if (AVG_0 >= 4.0)
		{
			printf("[CH0 : HIGH] ");
			Data.abs_fsr = 1;
		}
		else if (AVG_0 >= 2 && AVG_0<4)
		{
			printf("[CH0 : MID] ");
			Data.abs_fsr = 0;
		}
		else if (AVG_0<2)
		{
			printf("[CH0 : LOW]");
			Data.abs_fsr = -1;
		}
		printf("\n");
		///////////////////////////////////////////////////////////////////////////////ABS





		//////////////////////////////////////////////////////////////////////////////////
		if (AVG_4 >= 4.0)
		{
			printf("[CH4 : HIGH] ");
			Data.fsr1 = 1;
		}
		else if (AVG_4 >= 2 && AVG_4<4)
		{
			printf("[CH4 : MID] ");
			Data.fsr1 = 0;
		}
		else if (AVG_4<2)
		{
			printf("[CH4 : LOW]");
			Data.fsr1 = -1;
		}
		printf("\n");
		//////////////////////////////////////////////////////////////////////////////////FSR1


		//////////////////////////////////////////////////////////////////////////////////
		if (AVG_6 >= 4.0)
		{
			printf("[CH6 : HIGH] ");
			Data.fsr2 = 1;
		}
		else if (AVG_6 >= 2 && AVG_6<7)
		{
			printf("[CH6 : MID] ");
			Data.fsr2 = 0;
		}
		else if (AVG_6<2)
		{
			printf("[CH6 : LOW]");
			Data.fsr2 = -1;
		}

		printf("\n");
		//////////////////////////////////////////////////////////////////////////////////FSR2


		//////////////////////////////////////////////////////////////////////////////////
		if (AVG_7 >= 4.0)
		{
			printf("[CH7 : HIGH] ");
			Data.fsr3 = 1;
		}
		else if (AVG_7 >= 2 && AVG_7<4)
		{
			printf("[CH7 : MID] ");
			Data.fsr3 = 0;
		}
		else if (AVG_7<2)
		{
			printf("[CH7 : LOW]");
			Data.fsr3 = -1;
		}
		printf("\n");
		//////////////////////////////////////////////////////////////////////////////////FSR3



		//////////////////////////////////////////////////////////////////////////////////		
		if (night == 1)
		{
			if (AVG_23 >= 200.0)
			{
				printf("[CH23 : HIGH] ");
				Data.noise = 1;
			}
			else if (AVG_23 >= 60.0 && AVG_23<200.0)
			{
				printf("[CH23 : MID] ");
				Data.noise = 0;
			}
			else if (AVG_23<60.0)
			{
				printf("[CH23 : LOW]");
				Data.noise = -1;
			}

		}
		//////////////////////////////////////////////////////////////////////////////////SOUND

		if (flag == 1)
		{
			send(sock_desc,&Data,sizeof(Data),0);
			flag = 0;
		}
		else
			delay(100);
	}
	
}
void* RX_pthread(void* args)
{
	
	Time my_arg;
	int sock_desc = (int) args;
	
	for(;;)
	{

			recv(sock_desc,&my_arg, sizeof(Time),0);
			printf("month : %d, date: %d, hour: %d, min: %d, sec: %d\n", my_arg.mon, my_arg.day, my_arg.hour, my_arg.min, my_arg.sec);
			
			if (my_arg.hour >= 22 && my_arg.hour <= 5 || my_arg.isdt == 1)
			{	///SET NIGHT!!!
				night = 1;
				printf("NIGHT!!!!\n");
			}
			delay(1000);
			memset(&my_arg,0 ,sizeof(Time));

	}
	//pthread_mutex_unlock( &mutex1 );
}
void myInter()
{
	event++;
}
int read_mcp3208_adc(unsigned char adcChannel)
{
     unsigned char buff[3];
     int adcValue = 0;

     buff[0] = 0x06 | ((adcChannel & 0x07)>> 2);//1st byte
     buff[1] = ((adcChannel & 0x07)<<6);         //2nd byte
     buff[2] = 0x00;                                     //3nd byte
     //명령 비트를 통해 Channel을 선택함

     digitalWrite(CS_MCP3208,0); //Low : CS Active

     wiringPiSPIDataRW(SPI_CHANNEL,buff,3);

     buff[1] = 0x0F & buff[1];                 //1st byte
     adcValue = ( buff[1] << 8 ) | buff[2]; 
     //값을 읽어옴

     digitalWrite(CS_MCP3208,1);//High : CS Inactive
     return adcValue;

}
int main()
{
	printf("HAHAHAHAHAHAHAHAHA\n");
    int sock_desc;Time TIME;
    struct sockaddr_in serv_addr;
    
    pthread_t send_thread, recv_thread;
    

    
    if((sock_desc = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        printf("Failed creating socket\n");

    bzero((char *) &serv_addr, sizeof (serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("172.20.10.5");
    serv_addr.sin_port = htons(8000);
    
    
    
    
    if (connect(sock_desc, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) 
    {
        printf("Failed to connect to server\n");
        return -1;
    }
       printf("Connected successfully - Please enter string\n");

	 printf("HAHAHAHAHAHAHAHAHA\n");
	 system("clear");


      
      //ABS
      int adcChannel = SPI_CHANNEL;
      int adcValue = 0;
      double convertVol = 0;
      
      //FSR1
      int adcChannel4 = SPI_CHANNEL4;
      int adcValue4 = 0;
      double convertVol4 = 0;
      
      
      //FSR2
      int adcChannel6 = SPI_CHANNEL6;
      int adcValue6 = 0;
      double convertVol6 = 0;
      
      
      //FSR3
      int adcChannel7 = SPI_CHANNEL7;
      int adcValue7 = 0;
      double convertVol7 = 0;
      
      
     
     wiringPiSetupGpio();
     pinMode(CS_MCP3208,OUTPUT);
     pinMode(SOUND, INPUT);
     pinMode(BUTTON, INPUT);
     wiringPiISR(SOUND, INT_EDGE_FALLING, &myInter);
     
     
     if(wiringPiSetup() == -1)
     {
          fprintf(stdout,"Unable to startwiringPI : %s\n",strerror(errno));
          return 1;
     }

     if(wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1)
     {
          fprintf(stdout,"Unable to start wiringPI : %s\n",strerror(errno));
         return 1;
     }
     
     if(wiringPiSPISetup(SPI_CHANNEL4, SPI_SPEED) == -1)
     {
          fprintf(stdout,"Unable to start wiringPI : %s\n",strerror(errno));
         return 1;
     }
      
      if(wiringPiSPISetup(SPI_CHANNEL6, SPI_SPEED) == -1)
     {
          fprintf(stdout,"Unable to start wiringPI : %s\n",strerror(errno));
         return 1;
     }
      
      if(wiringPiSPISetup(SPI_CHANNEL7, SPI_SPEED) == -1)
     {
          fprintf(stdout,"Unable to start wiringPI : %s\n",strerror(errno));
         return 1;
     }
     
     
         
     pthread_create(&send_thread,NULL,TX_pthread, sock_desc);
     pthread_create(&recv_thread,NULL,RX_pthread, sock_desc);
     
   
     while(1)
     {
		 flag = 0;
		 Data.abs_fsr = 999;
		 Data.fsr1 = 999;
		 Data.fsr2 = 999;
		 Data.fsr3 = 999;
		 Data.noise = 999;
		 Data.bed_no = 21511864;
		 
		//CH0
        adcValue = read_mcp3208_adc(adcChannel);
        convertVol = ((double)adcValue / 4096) * 5;
        delay(10);
        //CH4
        adcValue4 = read_mcp3208_adc(adcChannel4);
        convertVol4 = ((double)adcValue4 / 4096) * 5;
        delay(10);
        //CH7
        adcValue6 = read_mcp3208_adc(adcChannel6);
        convertVol6 = ((double)adcValue6 / 4096) * 5;
        delay(10);
        //CH8
        adcValue7 = read_mcp3208_adc(adcChannel7);
        convertVol7 = ((double)adcValue7 / 4096) * 5;
        delay(10);
       
       
       
       
       
        if ((cnt == 1))
        {
			printf("   || [CH%d] ABS_fsr                [CH%d] FSR_1                  [CH%d] FSR_2                  [CH%d] FSR_3                     [CH23] SOUND_Sensor ", SPI_CHANNEL,SPI_CHANNEL4,SPI_CHANNEL6,SPI_CHANNEL7);
			
			
			printf("\n");
		}


		printf("%2d || [CH%d] Value = %4d, %4.2f(V)  [CH%d] Value = %4d, %4.2f(V)  [CH%d] Value = %4d, %4.2f(V)  [CH%d] Value = %4d, %4.2f(V) ",cnt,SPI_CHANNEL,adcValue, convertVol, SPI_CHANNEL4,adcValue4, convertVol4, SPI_CHANNEL6,adcValue6, convertVol6, SPI_CHANNEL7,adcValue7, convertVol7); 
		
		
		
		if ( night == 1 )
		{
			if (digitalRead(SOUND) == LOW)
			{

				delay(100);

				printf("    [CH23] event = %d\n",event);
				event =0;
			}
			else
			{
				delay(100);

				printf("    [CH23] event = %d\n",event);
				event =0;
			}
		}
		else if (night == 0 )
		{
			printf("    !!!Time's not Night!!!\n");
		}

		delay(250);
		
		
		pthread_mutex_lock( &mutex1 );

		AVG_0 += convertVol;
		AVG_4 += convertVol4;
		AVG_6 += convertVol6;
		AVG_7 += convertVol7;
		AVG_23 += event;
		
		pthread_mutex_unlock( &mutex1 );

		
		
		if (cnt == COUNT )
		{

			
			if (digitalRead(BUTTON) == HIGH)
			{
				Data.button = HIGH;
				printf("\n[BUTTON : HIGH]\n");
			}
			else if (digitalRead(BUTTON) == LOW)
			{
				Data.button = LOW;
				printf("\n[BUTTON : LOW]\n");
			}
			
		
			flag = 1;
			delay(100);
			printf("\n");
			getchar();
			system("clear");
			AVG_0 = 0;AVG_4 = 0;AVG_6 = 0;AVG_7 = 0;AVG_23 = 0;
			cnt = 0;
			event=0;
		
         memset(&TIME,0 ,sizeof(Time));
		}
		cnt++;
	  }
	  


 

    close(sock_desc);
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);
 
    return 0;

}

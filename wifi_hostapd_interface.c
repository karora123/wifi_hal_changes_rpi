#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include"wifi_hostapd_interface.h"
#include"wifi_hal.h"
#define wifi_printf printf
#define HOSTAPD_FNAME "/etc/hostapd"
#define SEC_FNAME "/etc/sec_file.txt"
#define RETURN_ERR -1
#define RETURN_OK 0
#define MAX_APS 2
#define CMD_SIZE 128
#define BUF_SIZE 128
static int read_hostapd();
char buf[50];
int read_hostapd(int ap,struct hostap_conf *conf);
int hostapd_write(int ap,struct params *params);
int hostapd_read(int ap,struct params *params,char *output);
struct  hostap_conf conf[MAX_APS];
int write_hostapd(int ap,struct params *params);
struct  params params;
int read_hostapd_all_aps();
#define STANDALONE_TEST 0
#undef STANDALONE_TEST
#if defined STANDALONE_TEST
int _syscmd(char *cmd, char *retBuf, int retBufSize)
{
    FILE *f;
    char *ptr = retBuf;
    int bufSize=retBufSize, bufbytes=0, readbytes=0;

    if((f = popen(cmd, "r")) == NULL) {
        printf("popen %s error\n", cmd);
        return RETURN_ERR;
    }

    while(!feof(f))
    {
        *ptr = 0;
        if(bufSize>=128) {
            bufbytes=128;
        } else {
            bufbytes=bufSize-1;
        }

        fgets(ptr,bufbytes,f);
        readbytes=strlen(ptr);
        if( readbytes== 0)
            break;
        bufSize-=readbytes;
        ptr += readbytes;
    }
    pclose(f);
    retBuf[retBufSize-1]=0;
    return RETURN_OK;
}


int main()
{
 
 int  ap=1;
 int i;
 char output[20];
//read_hostapd(ap,&conf);
read_hostapd_all_aps();

 for(i=0;i<MAX_APS;i++)
 {
     wifi_printf("\ni=%d\n",i);
    wifi_printf("\nssid[%d]=%s\n",i,conf[i].ssid);
    wifi_printf("\npassphrase[%d]=%s\n",i,conf[i].passphrase);
 } 
 printf("\n---------0------\n");
 strcpy(params.name,"ssid");
// strcpy(params.value,"MY_SSID_VALUE");
 hostapd_read(ap,&params,output);
 printf("\noutput=%s\n",output);
 printf("\n---------1------\n");
 strcpy(params.name,"wpa_passphrase");
 ap=1;
// strcpy(params.value,"MY_PASSPHRASE");
 hostapd_read(ap,&params,output);
 printf("\noutput=%s\n",output);
 strcpy(params.name,"ssid");
 strcpy(params.value,"ssid");
 hostapd_write(1,&params);
}
#endif

int hostapd_read(int ap,struct params *params,char *output)
{
    char file_name[20];
    char cmd[CMD_SIZE]={'\0'};
    char buf[BUF_SIZE]={'\0'};
    char *ch;
    char *pos;
    if(strcmp(params->name, "beaconType") == 0)
    {
        sprintf(cmd,"grep 'AP_SECMODE%d' %s",ap,SEC_FNAME);
        printf("\ncmd=%s\n",cmd);
    }
    else
    {
        sprintf(file_name,"%s%d.conf",HOSTAPD_FNAME,ap);
        sprintf(cmd,"grep '%s=' %s",params->name,file_name);
    }
    if(_syscmd(cmd,buf,sizeof(buf)) == RETURN_ERR)
    {
        wifi_printf("\nError %d:%s:%s\n",__LINE__,__func__,__FILE__);
        return RETURN_ERR;
    }
    if (buf[0]=='\0')
        return RETURN_ERR;
    pos = buf;
    while(*pos != '\0')
    {
        if (*pos == '\n')
        {
            *pos = '\0';
            break;
        }
        pos++;
    }
    pos = strchr(buf, '=');
    if (pos == NULL)
    {
        wifi_printf("Line %d: invalid line '%s'",__LINE__, buf);
        return RETURN_ERR;
    }
    *pos = '\0';//capture the value of the string
    pos++;
    strncpy(output,pos,strlen(pos));
}

int hostapd_write(int ap,struct params *params)
{
  char cmd[100];
  char wpa_val[2];
  char cur_val[100];
  char buff[100];
  if(strncmp(params->name,"beaconType",strlen("beaconType")) ==0 )
  {
      printf("\nparams name is BeaconType\n");
    if((strcmp(params->value,"WPAand11i")==0) || (strcmp(params->value,"11i")==0))
        strcpy(wpa_val,"2");
    else if((strcmp(params->value,"WPA")==0))
        strcpy(wpa_val,"1");        

    if(ap==0)
            strncpy(params->name,"AP_SECMODE0",strlen("AP_SECMODE0"));
    else if(ap==1)
        strncpy(params->name,"AP_SECMODE1",strlen("AP_SECMODE1"));
    else
        {
            wifi_printf("\n%s %d Invalid AP\n",__func__,__LINE__);
            return RETURN_ERR;
        }
    hostapd_read(ap,params,cur_val);
    sprintf(cmd,"sed -i 's/%s=%s/%s=%s/g' %s",params->name,cur_val,params->name,params->value,SEC_FNAME);
      _syscmd(cmd,buf,sizeof(buf));

    memset(params->name,'\0',sizeof(params->value));
    memset(params->value,'\0',sizeof(params->value));
    strncpy(params->name,"wpa",strlen("wpa"));
    strncpy(params->value,wpa_val,strlen(wpa_val));
    hostapd_read(ap,params,cur_val);
    sprintf(cmd,"sed -i 's/%s=%s/%s=%s/g' %s%d.conf",params->name,cur_val,params->name,params->value,HOSTAPD_FNAME,ap);
      _syscmd(cmd,buf,sizeof(buf));
    system("killall hostapd");
    sleep(1);
    system("hostapd -dd /etc/hostapd0.conf /etc/hostapd1.conf");
    return RETURN_OK;
  }
  
  hostapd_read(ap,params,cur_val);
  sprintf(cmd,"sed -i 's/%s=%s/%s=%s/g' %s%d.conf",params->name,cur_val,params->name,params->value,HOSTAPD_FNAME,ap);
  _syscmd(cmd,buf,sizeof(buf));
  wifi_printf("\ncmd=%s\n",cmd);
  system("killall hostapd");
  sleep(1);
  system("hostapd -B /etc/hostapd0.conf /etc/hostapd1.conf");
//  wifi_stopHostApd();
//  wifi_startHostApd();
  return RETURN_OK;
}


#ifdef USE_HOSTAPD_STRUCT

int read_hostapd(int ap,struct hostap_conf *conf)
{
    char *pos;
    FILE *f;
    int count=0;
    int line=0;
    char file_name[20];
    char cmd[CMD_SIZE]={'\0'};
    char buf[BUF_SIZE]={'\0'};
    char *ch;
    sprintf(file_name,"%s%d.conf",HOSTAPD_FNAME,ap);
    printf("\ncmdsssss=%s \n",file_name);
    f=fopen(file_name,"r");
    if (f == NULL) 
    {
        wifi_printf("Could not open configuration file '%s'for reading.",HOSTAPD_FNAME);
        return -1;
    }
    while (fgets(buf, sizeof(buf), f))
    {
        wifi_printf("\ncount=%d\n",count);
        count++;
        line++;
        /*Ignore comments*/
        if (buf[0] == '#') 
        continue;

        pos = buf;
        while (*pos != '\0') 
         {
              if (*pos == '\n') 
              {
                *pos = '\0';
                 break;
              }
              pos++;
        }
     
        if (buf[0]=='\0')
        continue;
        pos = strchr(buf, '=');
        if (pos == NULL) 
        {
               wifi_printf("Line %d: invalid line '%s'",__LINE__, buf);
              continue;
        }
        *pos = '\0';//capture the value of the string
        pos++;
        wifi_printf("\nname=%s value=%s\n",buf,pos);
        hostapd_fill(conf,buf,pos,line);
    }
    printf("ssid=%s",conf->ssid);
}

int read_hostapd_all_aps()
{
 int i;
 for(i=0;i<MAX_APS;i++)
 {
     wifi_printf("\ni=%d",i);
    printf("\nCalling read_hostapd\n");
     read_hostapd(i,&conf[i]);//fill the structure for both the hostapd configuration files
 }
}
hostapd_fill(struct hostap_conf *conf, char *buf, char *pos, int line)
{
    if(strcmp(buf,"ssid")==0)
    {
        memset(conf->ssid,'\0',sizeof(conf->ssid));
        strncpy(conf->ssid,pos,sizeof(conf->ssid));
    }
    else if(strcmp(buf,"wpa_passphrase")==0)
    {
        int len=strlen(pos);
        wifi_printf("\npass=%s\n",pos);

        if(len < 8 || len > 63)
        {
             wifi_printf("Line %d invalid Wpa Passphrase length %d \
                    expected (8..63) ",__LINE__,len);
        wifi_printf("\n4\n");
        }
        else
        {
            wifi_printf("\npass=%s\n",pos);
            free(conf->passphrase);
            conf->passphrase=strdup(pos);
        }
    }

}



int write_hostapd(int ap,struct params *params)
{
    char file_str[1024]={'\0'};
    FILE *f;
    char *pos;
    int line=0;
    int count=0;
    char file_name[20];
    sprintf(file_name,"%s%d.conf",HOSTAPD_FNAME,ap);
    wifi_printf("\nfile_name=%s\n",file_name);
    wifi_printf("\n%s\n",file_name);
    f=fopen(file_name,"r");
    if (f == NULL)
       {
           wifi_printf("Could not open configuration file '%s'for reading.",HOSTAPD_FNAME);
        return 0;
    }
    while (fgets(buf, sizeof(buf), f))
    {
        wifi_printf("\ncount=%d\n",count);
        count++;line++;
        pos = buf;
        while (*pos != '\0')
        {
            if (*pos == '\n')
            {
                *pos = '\0';
                break;
             }
             pos++;
         } 

         if (buf[0]=='\0')
             continue;
    
        pos = strchr(buf, '=');
          if (pos == NULL)
          {
              wifi_printf("Line %d: invalid line '%s'",__LINE__, buf);
              continue;
          }
          *pos = '\0';//capture the value of the string
          pos++;
//        wifi_printf("\n%s\n",buf);
//          write_hostapd(ap,params,buf,pos,line);
    }
    fclose(f);
    //refresh the conf structure 
    read_hostapd(ap,&conf[ap]);
}
#endif

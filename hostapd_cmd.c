#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include"hostapd_cmd.h"
#define wifi_printf printf
#define FNAME "hostapd"
#define RETURN_ERR -1
#define RETURN_OK 0
#define MAX_APS 2
static int read_hostapd();
char buf[50];
static hostapd_read(int ap,struct params *params);
int write_hostapd(int ap,struct params *params);
struct  hostap_conf conf[MAX_APS];
struct  params params;
int read_hostapd_all_aps();
#if defined HOSTAPD_TEST
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
 
 int  ap=0;
 int i;
//read_hostapd(ap,&conf);
read_hostapd_all_aps();

 for(i=0;i<MAX_APS;i++)
 {
 	printf("\ni=%d",i);
	printf("\nssid[%d]=%s\n",i,conf[i].ssid);
	printf("\npassphrase[%d]=%s\n",i,conf[i].passphrase);
 } 
/* printf("\n---------0------\n");
 strcpy(params.name,"ssid");
 strcpy(params.value,"MY_SSID_VALUE");

 write_hostapd(ap,&params);
 strcpy(params.name,"wpa_passphrase");
 strcpy(params.value,"MY_PASSPHRASE");

 printf("\n---------1------\n");
 */
// ap=1;
// write_hostapd(ap,&params);
// #endif
// read_hostapd(ap,&params);
// write_hostapd("wpa_passphrase","iii");
  	
}
#endif
static int hostapd_write(int ap,struct params *params,char *buf,char *pos,int line)
{
  char cmd[100];
  if(strcmp(buf,params->name)==0)
  {
	sprintf(cmd,"sed -i 's/%s=%s/%s=%s/g' hostapd%d.conf",params->name,pos,params->name,params->value,ap);
  	printf("\ncmd=%s\n",cmd);
    system(cmd);
  }
}

int read_hostapd_all_aps()
{
 int i;
 for(i=0;i<MAX_APS;i++)
 {
 	printf("\ni=%d",i);
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
        printf("\npass=%s\n",pos);

        if(len < 8 || len > 63)
        {
             wifi_printf("Line %d invalid Wpa Passphrase length %d \
                    expected (8..63) ",__LINE__,len);
        printf("\n4\n");
        }
        else
        {
            printf("\npass=%s\n",pos);
            free(conf->passphrase);
            conf->passphrase=strdup(pos);
        }
    }

}
#if 0
static hostapd_read(int ap,struct params *params)
{
  char cmd[50];
  char *str;
  char *str2;
  sprintf(cmd,"grep %s= ' /etc/hostapd%d.conf",params->name,ap);
  if(_syscmd(cmd,str,sizeof(cmd)))
  {
     wifi_printf("\nError %d:%s:%s\n",__LINE__,__func__,__FILE__);
     return RETURN_ERR;
  }
  
  str2=strchr(str,'=');
  str2='\0';
  str2++;
  printf("\nThe string is %s\n",str2);
}
#endif
static int read_hostapd(int ap,struct hostap_conf *conf)
{
	char *pos;
	FILE *f;
	int count=0;
	int line=0;
    char file_name[20];
	sprintf(file_name,"%s%d.conf",FNAME,ap);
	printf("\nfile_name=%s\n",file_name);
	f=fopen(file_name,"r");
	
	if (f == NULL) 
		{
	        wifi_printf("Could not open configuration file '%s'for reading.",FNAME);
	        return 0;
	    }
	 while (fgets(buf, sizeof(buf), f))
	 {
	 	
	 	printf("\ncount=%d\n",count);
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
	printf("\nname=%s value=%s\n",buf,pos);
	hostapd_fill(conf,buf,pos,line);
#if 0
	if(strcmp(buf,"ssid")==0)
	{
		memset(conf->ssid,'\0',sizeof(conf->ssid));
		strncpy(conf->ssid,pos,sizeof(conf->ssid));
	}
	if(strcmp(buf,"wpa_passphrase")==0)
	{
		int len=strlen(pos);
		printf("\npass=%s\n",pos);

		if(len < 8 || len > 63)
		{
			 wifi_printf("Line %d invalid Wpa Passphrase length %d \
		 			expected (8..63) ",__LINE__,len);
		printf("\n4\n");
		}
		else
		{
			printf("\npass=%s\n",pos);
			free(conf->passphrase);
			conf->passphrase=strdup(pos);
		}
	}
#endif
//	printf("\npassphrase=%s\n",conf->passphrase);
/*	if(strcmp(buf,"wpa_pairwise")
	if(strcmp(buf,"wpa");
	if(strcmp(buf,"wpa_keymgmt");;*/
}
//printf("ssid=%s",conf->ssid);
}

int write_hostapd(int ap,struct params *params)
{
	char file_str[1024]={'\0'};
    FILE *f;
	char *pos;
	int line=0;
    int count=0;
    char file_name[20];
    sprintf(file_name,"%s%d.conf",FNAME,ap);
    printf("\nfile_name=%s\n",file_name);
	printf("\n%s\n",file_name);
    f=fopen(file_name,"r");


    if (f == NULL)
        {
            wifi_printf("Could not open configuration file '%s'for reading.",FNAME);
            return 0;
        }
     while (fgets(buf, sizeof(buf), f))
     {
       printf("\ncount=%d\n",count);
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
//    printf("\n%s\n",buf);
      hostapd_write(ap,params,buf,pos,line);
    }
	fclose(f);
	#if 0

	if(strcmp(para_name,"ssid")==0)
	{
    	memset(conf->ssid,'\0',sizeof(conf->ssid));
	    strncpy(conf->ssid,value,sizeof(conf->ssid));
	}
	else if(strcmp(para_name,"wpa_passphrase")==0)
    {
		free(conf->passphrase);
		conf->passphrase=strdup(value);
	}
	#endif
	
//	sprintf(file_str,"cat > hostapd.conf <<EOF\nssid=%s\nwpa_passphrase=%s\nEOF",conf->ssid,conf->passphrase);
//	printf("\n%s\n",file_str);
//	system(file_str);
	//refresh the 
	read_hostapd(ap,&conf[ap]);
}

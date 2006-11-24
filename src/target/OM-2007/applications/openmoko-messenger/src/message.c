#include "message.h"

int get_folder_message_number(char *fname)
{
  int num=0;
	
  FILE *fp;
  MESSAGE msg;
  char next;

  /* display the result change */
  fp = fopen("message.dat","r");
  
  fseek(fp,0,SEEK_SET);
  while(1) 
    {
      fread(&msg,sizeof(msg),1,fp);
      if(!strcmp(msg.folder,fname))
		num ++;
      next = fgetc(fp);
      fseek(fp,-1,SEEK_CUR);
      if(next == EOF)
		break;
    }
  
  fclose(fp);

  return num;
}

int message_empty()
{
  int is_empty = 1;

  FILE *fp;
  MESSAGE msg;
  char next;

  /* display the result change */
  fp = fopen("message.dat","r");
  
  fseek(fp,0,SEEK_SET);
  while(1) 
    {
      fread(&msg,sizeof(msg),1,fp);
      if(msg.id != -2)
		is_empty = 0;
      next = fgetc(fp);
      fseek(fp,-1,SEEK_CUR);
      if(next == EOF)
		break;
    }
  
  fclose(fp);

  return is_empty;
}

void set_msg_status(int id, STATUS st)
{
  printf("message id %d\n",id);

  FILE *fp;
  MESSAGE msg, *mapped;
  char next;
  int n,f;

  /* get the whole number */
  fp = fopen("message.dat","r+");
  n=0;
  fseek(fp,0,SEEK_SET);
  while(1) 
    {
      fread(&msg,sizeof(msg),1,fp);
      n++;
      next = fgetc(fp);
      fseek(fp,-1,SEEK_CUR);
      if(next == EOF)
	break;
    }
  fclose(fp);
  
  f = open("message.dat",O_RDWR);
  mapped = (MESSAGE *)mmap(0, n*sizeof(msg), 
			   PROT_READ|PROT_WRITE, MAP_SHARED, f, 0);
  
  mapped[id-1].status=st;
    
  msync((void *)mapped, n*sizeof(msg), MS_ASYNC);
  munmap((void *)mapped, n*sizeof(msg));
  close(f);
  
}

void set_msg_folder(int id, char *fname)
{
  FILE *fp;
  MESSAGE msg, *mapped;
  char next;
  int n,f;

  /* get the whole number */
  fp = fopen("message.dat","r+");
  n=0;
  fseek(fp,0,SEEK_SET);
  while(1) 
    {
      fread(&msg,sizeof(msg),1,fp);
      n++;
      next = fgetc(fp);
      fseek(fp,-1,SEEK_CUR);
      if(next == EOF)
	break;
    }
  fclose(fp);
  
  f = open("message.dat",O_RDWR);
  mapped = (MESSAGE *)mmap(0, n*sizeof(msg), 
			   PROT_READ|PROT_WRITE, MAP_SHARED, f, 0);
  g_debug("Move message %d from %s to %s\n",id,mapped[id-1].folder,fname);
  strcpy(mapped[id-1].folder,fname);
  msync((void *)mapped, n*sizeof(msg), MS_ASYNC);
  munmap((void *)mapped, n*sizeof(msg));
  close(f);
  
}

int delete_msg(int msgid)
{
  FILE *fp;
  MESSAGE msg, *mapped;
  char next;
  int m,n,f;

  /* get the position and the whole number */
  fp = fopen("message.dat","r+");
  m=n=0;
  fseek(fp,0,SEEK_SET);
  while(1) 
    {
      fread(&msg,sizeof(msg),1,fp);
      n++;
      if(msg.id == msgid)
		m=n;
      next = fgetc(fp);
      fseek(fp,-1,SEEK_CUR);
      if(next == EOF)
	break;
    }
  printf("m=%d , n=%d\n",m,n);
  fclose(fp);
  
  f = open("message.dat",O_RDWR);
  mapped = (MESSAGE *)mmap(0, n*sizeof(msg), 
			   PROT_READ|PROT_WRITE, MAP_SHARED, f, 0);
  
  mapped[m-1].id=-2;
    
  msync((void *)mapped, n*sizeof(msg), MS_ASYNC);
  munmap((void *)mapped, n*sizeof(msg));
  close(f);
  
  return 1;
}

void add_msg(char *from, char *content, char *time, char *folder)
{
  FILE *fp;
  MESSAGE msg;
  int m=1;
  char next;

  /* get the whole number */
  fp = fopen("message.dat","r+");
  fseek(fp,0,SEEK_SET);

  while(1) 
    {
      next = fgetc(fp);
      fseek(fp,-1,SEEK_CUR);
      if(next == EOF)
	break;
      fread(&msg,sizeof(msg),1,fp);
      m++;
    }

  printf("messages has %d \n",m);
  fclose(fp);

  /* add the one */
  fp = fopen("message.dat","r+");
      
  fseek(fp,0,SEEK_END);
  msg.id = m;
  strcpy(msg.from,from);
  strcpy(msg.content,content);
  strcpy(msg.time,time);
  msg.status = ST_UNREAD;
  strcpy(msg.folder,folder);

  fwrite(&msg,sizeof(msg),1,fp);
  
  fclose(fp);
  
}

void msg_display()
{
  FILE *fp;
  MESSAGE msg;
  char next;

  /* display the result change */
  fp = fopen("message.dat","r");
  
  fseek(fp,0,SEEK_SET);
  while(1) 
    {
      fread(&msg,sizeof(msg),1,fp);
      printf("%d %s %s %s %d %s\n", msg.id, msg.from, msg.content, msg.time,
	     msg.status, msg.folder);

      next = fgetc(fp);
      fseek(fp,-1,SEEK_CUR);
      if(next == EOF)
	break;
    }
  
  fclose(fp);
}

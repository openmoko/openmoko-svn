#include "folder.h"

int custom_folder_in()
{
  FILE *fp;
  FOLDER folder;
  char next;
  int m=0;

  /* get the position and the whole number */
  fp = fopen("folder_list.dat","r+");

  fseek(fp,0,SEEK_SET);
  while(1) 
    {
      fread(&folder,sizeof(folder),1,fp);
      if(folder.type)
		m=1;
      next = fgetc(fp);
      fseek(fp,-1,SEEK_CUR);
      if(next == EOF)
	break;
    }
  fclose(fp);

  return m;
}

int is_custom_folder (char *fname)
{
  FILE *fp;
  FOLDER folder;
  char next;
  int m=0;

  /* get the position and the whole number */
  fp = fopen("folder_list.dat","r+");

  fseek(fp,0,SEEK_SET);
  while(1) 
    {
      fread(&folder,sizeof(folder),1,fp);
      if(!strcmp(folder.fname,fname))
		m=folder.type;
      next = fgetc(fp);
      fseek(fp,-1,SEEK_CUR);
      if(next == EOF)
	break;
    }
  fclose(fp);

  return m;
}

int delete_folder(char *dfname)
{
  FILE *fp;
  FOLDER folder, *mapped;
  char next;
  int m,n,f;

  /* get the position and the whole number */
  fp = fopen("folder_list.dat","r+");
  m=n=0;
  fseek(fp,0,SEEK_SET);
  while(1) 
    {
      fread(&folder,sizeof(folder),1,fp);
      n++;
      if(!strcmp(folder.fname,dfname))
	m=n;
      next = fgetc(fp);
      fseek(fp,-1,SEEK_CUR);
      if(next == EOF)
	break;
    }
  printf("m=%d , n=%d\n",m,n);
  fclose(fp);
  
  f = open("folder_list.dat",O_RDWR);
  mapped = (FOLDER *)mmap(0, n*sizeof(folder), 
                          PROT_READ|PROT_WRITE, MAP_SHARED, f, 0);
  
  strcpy(mapped[m-1].fname,"deleted");
  mapped[m-1].type = 0;
    
  msync((void *)mapped, n*sizeof(folder), MS_ASYNC);
  munmap((void *)mapped, n*sizeof(folder));
  close(f);
  
  return 1;
}

void add_folder(char *fname)
{
  FILE *fp;
  FOLDER folder;

  fp = fopen("folder_list.dat","r+");
    
  fseek(fp,0,SEEK_END);
  folder.type = 1;
  strcpy(folder.fname,fname);
  fwrite(&folder,sizeof(folder),1,fp);
  
  fclose(fp);
}

void rename_folder(char *fname, char *new_name)
{
  FILE *fp;
  FOLDER folder, *mapped;
  char next;
  int m,n,f;

  /* get the position and the whole number */
  fp = fopen("folder_list.dat","r+");
  m=n=0;
  fseek(fp,0,SEEK_SET);
  while(1) 
    {
      fread(&folder,sizeof(folder),1,fp);
      n++;
      if(!strcmp(folder.fname,fname))
	m=n;
      next = fgetc(fp);
      fseek(fp,-1,SEEK_CUR);
      if(next == EOF)
	break;
    }

  fclose(fp);
  
  f = open("folder_list.dat",O_RDWR);
  mapped = (FOLDER *)mmap(0, n*sizeof(folder), 
                          PROT_READ|PROT_WRITE, MAP_SHARED, f, 0);
  
  strcpy(mapped[m-1].fname,new_name);
    
  msync((void *)mapped, n*sizeof(folder), MS_ASYNC);
  munmap((void *)mapped, n*sizeof(folder));
  close(f);
  
}

void display()
{
  FILE *fp;
  FOLDER folder;
  char next;

  /* display the result change */
  fp = fopen("folder_list.dat","r");
  
  fseek(fp,0,SEEK_SET);
  while(1) 
    {
      fread(&folder,sizeof(folder),1,fp);
      printf("%s\n", folder.fname);
      /* get next char, if it's EOF break */
      next = fgetc(fp);
      fseek(fp,-1,SEEK_CUR);
      if(next == EOF)
	break;
    }
  
  fclose(fp);
}

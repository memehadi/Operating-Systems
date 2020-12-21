/* Musab Mehadi
mmehadi@jacobs-university.de*/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main (int argc, char **argv) {
  
  char c;
  int file_copy_type = 1;

  while ((c = getopt (argc, argv, "ls")) != -1)
  if(c=='l')
  {
    file_copy_type = 1;
    break;
  }
  else if(c=='s')
  {
    file_copy_type = 2;
    break;
  }
  else
  {
    file_copy_type = 1;
    break;
  }   

  char src[32] = "";
  char tar[32] = "";
  char buffer[32] = "";

  printf(" Please enter name of source file: ");
  scanf("%s", src );

  printf("Please enter the name of target file: ");
  scanf("%s", tar );

  if ( file_copy_type == 1 )
  {

    // opening file in read mode and checking
      FILE *f_src = fopen( src , "r");
      
      if( f_src == NULL ){
        printf("Error,couldn't read the file : %s\n", src);
        return 1;
      }

      // opening file and checking
      FILE *f_tar = fopen( tar , "w" );

      if( f_tar == NULL ){
        printf("Error,couldn't' write on the file %s\n", tar);
        fclose(f_src);
        return 1;
      }
  
      // copy file source  to target 
      char ch;
      while( ( ch = getc(f_src) ) != EOF )
        putc(ch, f_tar);

      fclose(f_src);
      fclose(f_tar);

      printf("File copied succesfully(using c library)\n");

  }
  else if ( file_copy_type == 2 )
  {
      // opening file in read mode and checking
      int f_src = open( src ,O_RDONLY);

      if( f_src == -1 ){
        printf("Error,couldn't read the file : %s\n", src);
        return 1;
      }

      // opening file in read write mode and checking
      int f_tar = open( tar ,O_CREAT | O_RDWR );

      if( f_tar == -1 ){
        printf("Error,couldn't write the file %s\n", tar);
        close(f_src);
        return 1;
      }

      // copy file source  to target 
      while ( 1 )
      {
        int bytes = read ( f_src, buffer, 1 ) ;

        if ( bytes > 0 )
          write ( f_tar, buffer, bytes ) ;
        else
          break ;
      }

      close(f_src);
      close(f_tar);

      printf("File copied succesfully (using system calls)\n");

  }

  return 0;
}

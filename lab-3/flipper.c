#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <dirent.h>

char SOURCE_DIR[1000];
char OUTPUT_DIR[1000];
void revertLine(char * text, size_t size){
    for(int i=0;i<size/2;i++){
        char temp = text[i];
        text[i]=text[size-i-1];
        text[size-i-1]=temp;
    }
    text[size] = '\n';
}
size_t readFile(FILE * file,char ** text){
    size_t size = 0;
    size_t capacity = 1024;
    char *buffer = malloc(capacity);
    char curr;
    size_t line_len=0;
    while(!feof(file)){
        int el = fread(&curr,sizeof(char),1,file);
        if(el==0){
            break;
        }
        if (size >= capacity - 1) {
            capacity *= 2;
            char *new_buffer = realloc(buffer, capacity);
            buffer = new_buffer;
        }
        if(curr=='\n'){
            revertLine(&buffer[size-line_len],line_len);
            line_len=-1;
         }
        buffer[size++]=curr;
        line_len++;
    }
    buffer[size]='\0';
    *text = buffer;
    return size;
}
int proccessFile(const char * path,const struct stat * sb, int typeflag, struct FTW * ftwbuf){
    char * text= NULL;

    const char *fileName = strrchr(path, '/');
    if (fileName) {
        fileName++;
    } else {
        fileName = path;
    }
    if(strncmp(path + strlen(path) - 4, ".txt", 4) != 0){
        printf("File %s not processed\n",path);
        return 0;
    }
    FILE * file = fopen(path,"r");
    if (!file) {
        perror("Nie udało się otworzyć pliku");
        return 0;
    }
    // size_t size = fread(text,sizeof(char),sizeof(text) - 1,file);
    size_t size =readFile(file,&text);
    fclose(file);

    // writing to file
    char out_path[1024];
    memset(out_path, '\0', sizeof(out_path));
    strcat(out_path,OUTPUT_DIR);
    strcat(out_path,"/");
    strcat(out_path,fileName);
    printf("Output path %s\n",out_path);

    FILE * modified_file = fopen(out_path,"a+");
    fseek(modified_file, 0, SEEK_SET);
    fwrite(text,sizeof(char),size,modified_file);

    fclose(modified_file);
    free(text);

    return 0;
}
void searchInCatalogs(const char *dir){
    if (nftw(dir, proccessFile, 20, 0) == -1) {
        perror("nftw");
        exit(EXIT_FAILURE);
    }
}
int main(){
    printf("Podaj ścieżke do katalogu źródłowego: ");
    scanf("%s",SOURCE_DIR);
    printf("Podaj ścieżke do katalogu wyjściowego: ");
    scanf("%s",OUTPUT_DIR);

    DIR *dir = opendir(OUTPUT_DIR);
    if (dir) {
        closedir(dir);
    } else {
        printf("Creating output directory: %s\n", OUTPUT_DIR);
        if (mkdir(OUTPUT_DIR, 0755) != 0) {
            perror("Failed to create output directory");
            return 1;
        }
    }

    searchInCatalogs(SOURCE_DIR);
}
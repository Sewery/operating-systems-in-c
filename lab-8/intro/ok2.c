#define ROZM_BLOKU 1024
int main(int argc, char ** argv){
    int fd = shm_open("/nazwa_pam",O_CREAT | O_RDWR, 8644);
    // utworzenie segmentu pam.wspolne
    ftruncate(fd,ROZM_BLOKU);
    // dolaczenie segmentu, pamieci, do przestrzeni adresowej procesu:
    char *wsk = (char *)mmap(NULL,ROZM_BLOKU,PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    strcpy(wsk,"dana do zapisania w pamieci wspolnej");
    munmap(wsk,ROZM_BLOKU);//odlaczanie segmentu od przestrzenie adrosowej procesu
    // shm_unlink("/nazwa_pem"); // oznaczenie segmentu do usunecia
    return 0; 


}
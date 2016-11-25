int palindromo(char* path){
    char* control=path;
    char* app=path;
    int n=0;
    int inizio;
    int fine;
    int pal;
    int i=0;
    
    if (path==NULL) {
        return 0;
    }
    
    else{
        
        while ((*control)!='\0'){
            n++;
            control++;
        }
        
        if (n==0) {
            return 1;
        }
        else{//inizio controllo della stringa palindroma
            inizio=0;
            fine=n;
            pal=1;
            
            while (pal==1 && i<=(inizio+fine)/2) {
                i++;
                if (app[inizio]!=app[fine-1]){
                    pal=0;
                }
                
            }
            if (pal==1) {
                return 1;
            }
            
        }
        
    }
    
    return 0 ;
}

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    FILE *inFile;
    FILE *outFile;
    int matchFlag1 = 0;//match ""
    int matchFlag2 = 0;//match {}
    int matchFlag3 = 0;//match /**/
    int matchFlag4 = 0;//match // and \r\n
    char ch, chNext;
   
    if(argc < 3)
    {
        printf("please give two file names for input and output\n");
        printf("you can use command: ./run arg1 arg2\n");
        return -1;
    }
    
    printf("input file is: %s\n", argv[1]);
    printf("output file is: %s\n", argv[2]);
    
    inFile = fopen(argv[1], "r");
    if(inFile == NULL)
    {
        printf("can't open input file: %s", argv[1]);
        return -1;
    }
    
    outFile = fopen(argv[2], "w+");
    if(outFile == NULL)
    {
        printf("can't open output file: %s", argv[2]);
        return -1;
    }

    while(EOF != (ch = fgetc(inFile)))
    {
        if(ch == '\"')
        {
            if(!matchFlag1)//the first '\"' 
                matchFlag1 = 1;
            else
                matchFlag1 = 0;
        }
        if(!matchFlag1)//filter except the content in the ""
        {
            if(ch == '{')
            {
                if(!matchFlag2)
                    fputc(ch, outFile);//write the '{'
                matchFlag2++;
            }
            else if(ch == '}')
                matchFlag2--;
            if(!matchFlag2)
            {
                if(ch == '/' && !matchFlag3 && !matchFlag4)
                {
                    chNext = fgetc(inFile);
                    if(chNext == '*')// it is "/*"
                        matchFlag3 = 1;
                    else if(chNext == '/')//it is "//"
                        matchFlag4 = 1;
                    else
                    {
                        fputc(ch, outFile);//write the '/'
                        ch = chNext;
                    }    
                }
                if(matchFlag3 && ch == '*')
                {
                    chNext = fgetc(inFile);
                    if(chNext == '/')
                    {
                        matchFlag3 = 0;
                        continue;
                    }
                    else
                    {
                        fputc(ch, outFile);//write the '*'
                        ch = chNext;
                    }    
                }
                if(matchFlag4 && ch == '\r')//dos file format "\r\n"
                {
                    chNext = fgetc(inFile);
                    if(chNext == '\n')
                    {
                        matchFlag4 = 0;
                    }
                    fputc(ch, outFile);//write the '\r'
                    ch = chNext;
                }
            }
        }
        if(matchFlag2 || matchFlag3 || matchFlag4)
            continue;//filter the content in the {}, /**/, //

        fputc(ch, outFile);

    }

    fclose(inFile);
    fclose(outFile);
    return 0;
}

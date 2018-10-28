#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <math.h>

#define REVCOUNT 25000 //TODO MAKE THIS 25000
#define LINELEN 16*1024

typedef struct _kv{
    int k;
    int v;
    struct _kv *next;
}kv;

typedef struct review{
    int class;  //this is my calculated review - depends on if posRevProv or negRevProb is higher
    int label; //this is the given review
    int posRevProb; // sum of all log(posWordProb) in this review    
    int negRevProb; // sum of all log(negWordProb) in this review
    kv *firstWord;
}Review;

Review* train_reviews[REVCOUNT];    
Review* test_reviews[REVCOUNT];    
float posWordsFreq[100000];
float negWordsFreq[100000];

float posWordsProb[100000];
float negWordsProb[100000];

void lineParser(char* file, Review**);
void printReview(Review*);
unsigned int posWordCounter(Review*);
unsigned int negWordCounter(Review*);
void probCalc(Review** rev, int totalNegWords, int totalPosWords);
float error(Review** rev);

int main(void){
    
    unsigned int totalPosWords = 0, totalNegWords = 0;
    for (int i =0;i<100000;i++){
        posWordsFreq[i]=1;
        negWordsFreq[i]=1;
        posWordsProb[i]=1;
        negWordsProb[i]=1;
    }

    lineParser("labeledBow_train.feat", (Review **)&train_reviews);       
    for(size_t i = 0; i < REVCOUNT; ++i){
        //  printReview(train_reviews[i]);
        totalPosWords += posWordCounter(train_reviews[i]);
        totalNegWords += negWordCounter(train_reviews[i]);
    }
    totalPosWords += 10000; //this is part of the laplace function
    totalPosWords += 10000; //this is part of the laplace function
    printf("total Pos words: %u\n", totalPosWords);
    printf("total Neg words: %u\n", totalNegWords);
    probCalc((Review **)&train_reviews, totalPosWords, totalNegWords);
    printf("training error: %f%%\n",error((Review **)&train_reviews)*100);

    lineParser("labeledBow_test.feat", (Review **)&test_reviews);
    probCalc((Review **)&test_reviews, totalPosWords, totalNegWords);
    printf("testing error: %f%%\n",error((Review **)&test_reviews)*100);
    

    return 0;
}

float error(Review** rev){
    float errorSum=0;
    for(size_t i = 0; i < REVCOUNT; ++i){
        Review* r = rev[i];
        if(r->label != r->class){
            ++errorSum;
        }
    }
    return errorSum/REVCOUNT;
}

void probCalc(Review** rev, int totalNegWords, int totalPosWords){
    for (int i =0;i<100000;i++){
        posWordsProb[i] = logf(posWordsFreq[i]/totalPosWords);
        negWordsProb[i] = logf(negWordsFreq[i]/totalNegWords);
    }
    for(size_t i = 0; i < REVCOUNT; ++i){
        Review* r = rev[i];
        kv* pair = r->firstWord;
        while(pair != NULL){
            r->negRevProb += negWordsProb[pair->k];
            r->posRevProb += posWordsProb[pair->k];
            pair = pair->next;
        }
        if(r->posRevProb > r->negRevProb){
            r->class = 1;
        }
        else{
            r->class = 0;
        }


    }
}
unsigned int posWordCounter(Review* r){
    unsigned int totalPos = 0;          
    kv* pair = r->firstWord;
    if(r->label == 1){
        while(pair != NULL){
            totalPos += pair->v;
            posWordsFreq[pair->k]++;
            
            pair = pair->next;
        }
    }
    return totalPos;
}

unsigned int negWordCounter(Review* r){
    unsigned int totalNeg = 0;
    kv* pair = r->firstWord;
    if(r->label == 0){
        while(pair != NULL){
            totalNeg += pair->v;
            negWordsFreq[pair->k]++;
            pair = pair->next;
        }
    }
    return totalNeg;
}

void printReview(Review* r){
    printf("label: %d", r->label);
    kv* pair = r->firstWord;
    while(pair != NULL){
        printf("k = %d, v = %d\n", pair->k, pair->v);
        pair = pair->next;
        }
    }

void lineParser(char* file_name, Review** rev){
    FILE *ifp;
    char line[LINELEN];
    char* mode = "r";
    ifp = fopen(file_name, mode);
    if(ifp == NULL){
        fprintf(stderr, "Can't open input file!\n");
        exit(1);
    }
    for(int i = 0; i<REVCOUNT; ++i){
        if (fgets(line, sizeof(line), ifp)==NULL){
            fprintf(stderr, "Invalid line.\n");
        }
        Review* r = malloc(sizeof(Review));
        rev[i] = r;
        //printf("%d\n", r->label);  //This is how you view the label of each review
        char* tok = strtok(line, " :"); //splits line using delimiter space and colon
        r->label = atoi(tok);
        if(r->label >5){
            r->label = 1;
        }    
        else{                
            r->label = 0;
        }
        //printf("label: %d\n", r->label);  //This is how you view the label of each review
        kv* pair = r->firstWord = malloc(sizeof(kv));
        while(tok != NULL){
            tok = strtok(NULL, " :");
            if(tok){
                pair->k = atoi(tok);
                //printf("k = %d", pair->k);
            }
            else{
                pair->next = NULL;
                break;
            }
            tok = strtok(NULL, " :");
            if(tok){
                pair->v = atoi(tok);
                //printf("v = %d", pair->v);
            }
            else{
                pair->next = NULL;
                break;
            }
            pair->next = malloc(sizeof(kv));
            pair = pair->next;
        }
        pair->next = NULL;
        //puts("---");
    }
    fclose(ifp);
}

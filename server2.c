#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <netinet/in.h>

int num_students;

typedef struct student
{
    char lname[10];
    char initial;
    char fname[10];
    unsigned long SID;
    float GPA;
    struct student *next;
} SREC;

SREC *head = NULL;

SREC *createStudent(char lname[], char initial, char fname[], unsigned long SID, float GPA)
{
    SREC *newStudent = (SREC *)malloc(sizeof(SREC));

    newStudent->next = NULL;

    strncpy(newStudent->lname, lname, 10);
    newStudent->initial = initial;
    strncpy(newStudent->fname, fname, 10);
    newStudent->SID = SID;
    newStudent->GPA = GPA;

    num_students += 1;

    return newStudent;
}

void insert(char lname[], char initial, char fname[], unsigned long SID, float GPA)
{
    SREC *newStudent = createStudent(lname, initial, fname, SID, GPA);
    SREC *temp = head;
    if (head == NULL)
    {
        head = newStudent;
        return;
    }

    while (temp->next != NULL)
    {
        temp = temp->next;
    }
    temp->next = newStudent;
}

void delete(unsigned long deleteSID)
{
    SREC *current = head;
    SREC *previous = head;

    if (head != NULL && deleteSID == head->SID)
    {
        head = head->next;
        free(current);
        num_students -= 1;
        return;
    }

    while (current != NULL && deleteSID != current->SID)
    {
        previous = current;
        current = current->next;
    }

    if (current != NULL)
    {
        previous->next = current->next;

        num_students -= 1;

        free(current);
    }
}

int compareLname(const void *x, const void *y)
{
    SREC *student_1 = (SREC *)x;
    SREC *student_2 = (SREC *)y;

    int result = strcasecmp(student_1->lname, student_2->lname);
    if (result != 0)
    {
        return result;
    }

    return (strcmp(student_1->lname, student_2->lname));
}

int compareFname(const void *x, const void *y)
{
    SREC *student_1 = (SREC *)x;
    SREC *student_2 = (SREC *)y;

    int result = strcasecmp(student_1->fname, student_2->fname);
    if (result != 0)
    {
        return result;
    }

    return (strcmp(student_1->fname, student_2->fname));
}

int compareSID(const void *x, const void *y)
{
    SREC *student_1 = (SREC *)x;
    SREC *student_2 = (SREC *)y;

    if (student_1->SID < student_2->SID)
    {
        return -1;
    }
    else if (student_1->SID == student_2->SID)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int compareGPA(const void *x, const void *y)
{
    SREC *student_1 = (SREC *)x;
    SREC *student_2 = (SREC *)y;

    if (student_1->GPA < student_2->GPA)
    {
        return 1;
    }
    else if (student_1->GPA == student_2->GPA)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

SREC *get(int order)
{
    SREC *tmp = head;
    SREC *emptyList = malloc(sizeof(SREC) * num_students);
    int studentCount = 0;

    while (tmp != NULL)
    {
        strncpy(emptyList[studentCount].lname, tmp->lname, 10);
        emptyList[studentCount].initial = tmp->initial;
        strncpy(emptyList[studentCount].fname, tmp->fname, 10);
        emptyList[studentCount].SID = tmp->SID;
        emptyList[studentCount].GPA = tmp->GPA;
        tmp = tmp->next;
        studentCount++;
    }

    if (order == 1)
    {
        qsort(emptyList, num_students, sizeof(SREC), compareLname);
    }
    else if (order == 2)
    {
        qsort(emptyList, num_students, sizeof(SREC), compareFname);
    }
    else if (order == 3)
    {
        qsort(emptyList, num_students, sizeof(SREC), compareSID);
    }
    else if (order == 4)
    {
        qsort(emptyList, num_students, sizeof(SREC), compareGPA);
    }

    return emptyList;
}

int main(int argc, char *argv[])
{
    int server_socket, client_socket, portnum;
    struct sockaddr_in server_address;

    int length;

    char *token;

    char *tokenBuffer[2];
    int tokenCounter = 0;

    char *infoBuffer[5];
    int infoCounter = 0;

    char lname[10], fname[10];
    char initial;
    unsigned long SID;
    float GPA;

    unsigned long deleteSID;

    int readFile = 0;

    SREC *tempList;

    SREC *tmp;

    FILE *fptrin;
    FILE *fptrout;

    int i;

    char dataBuffer[256];

    char buffer[256];

    char secret[256] = "end";

    char invalidInput[256] = "Invalid input";

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(5);
    }

    portnum = atoi(argv[1]);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0)
    {
        fprintf(stderr, "ERROR opening socket\n");
        exit(1);
    }

    memset((char *)&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portnum);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        fprintf(stderr, "ERROR binding socket\n");
        exit(2);
    }

    if (listen(server_socket, 5) < 0)
    {
        fprintf(stderr, "ERROR listening on socket\n");
        exit(3);
    }

    memset(buffer, 0, 256);

    for (;;)
    {

        client_socket = accept(server_socket, NULL, NULL);

        if (client_socket < 0)
        {
            fprintf(stderr, "ERROR accepting connection\n");
            exit(4);
        }

        if (readFile == 0)
        {
            readFile = 1;

            fptrin = fopen("db.out", "r");

            if (fptrin != NULL)
            {

                tmp = (SREC *)calloc(sizeof(SREC), 1);

                while (fread(tmp, sizeof(SREC), 1, fptrin) == 1)
                {

                    strncpy(lname, tmp->lname, 10);
                    initial = tmp->initial;
                    strncpy(fname, tmp->fname, 10);
                    SID = tmp->SID;
                    GPA = tmp->GPA;

                    insert(lname, initial, fname, SID, GPA);
                }
            }
            else
            {
                fprintf(stderr, "Error opening file\n");
            }

            fclose(fptrin);
        }

        for (;;)
        {
            length = recv(client_socket, &buffer, sizeof(buffer) - 1, 0);

            if (length < 1)
            {

                fptrout = fopen("db.out", "wb");

                if (fptrout != NULL)
                {
                    tmp = head;

                    while (tmp != NULL)
                    {
                        fwrite(tmp, sizeof(SREC), 1, fptrout);

                        tmp = tmp->next;
                    }

                    fclose(fptrout);
                }
                else
                {
                    fprintf(stderr, "Error opening file\n");
                }

                break;
            }

            buffer[length] = '\0';

            if (length >= sizeof(buffer))
            {
                printf("Message exceeds the buffer size\n");
            }

            token = strtok(buffer, " ");

            tokenBuffer[0] = '\0';
            tokenCounter = 0;

            infoBuffer[0] = '\0';
            infoCounter = 0;

            while (token != NULL)
            {

                tokenBuffer[tokenCounter] = token;
                tokenCounter++;

                token = strtok(NULL, " ");
            }

            if (strcmp(tokenBuffer[0], "put") == 0)
            {

                token = strtok(tokenBuffer[1], ",");

                while (token != NULL)
                {

                    infoBuffer[infoCounter] = token;
                    infoCounter++;

                    token = strtok(NULL, ",");
                }

                if (infoCounter != 5)
                {
                    printf("incorrect number of information");
                }

                strncpy(lname, infoBuffer[0], 10);
                initial = infoBuffer[1][0];
                strncpy(fname, infoBuffer[2], 10);
                SID = strtol(infoBuffer[3], NULL, 10);
                GPA = (float)atof(infoBuffer[4]);

                insert(lname, initial, fname, SID, GPA);
            }
            else if (strcmp(tokenBuffer[0], "get") == 0)
            {

                if (strcmp(tokenBuffer[1], "lname\n") == 0)
                {

                    tempList = get(1);

                    strncpy(dataBuffer, "| SID   | Lname     | Fname      | M | GPA  |", 256);
                    send(client_socket, dataBuffer, sizeof(dataBuffer), 0);
                    dataBuffer[256] = '\0';
                    strncpy(dataBuffer, "+-------+-----------+------------+---+------+", 256);
                    send(client_socket, dataBuffer, sizeof(dataBuffer), 0);
                    dataBuffer[256] = '\0';
                    for (i = 0; i < num_students; i++)
                    {
                        sprintf(dataBuffer, "| %-5ld | %-9s | %-9s  | %c | %.2f |", tempList[i].SID, tempList[i].lname, tempList[i].fname, tempList[i].initial, tempList[i].GPA);
                        send(client_socket, dataBuffer, sizeof(dataBuffer), 0);
                        dataBuffer[256] = '\0';
                    }
                    strncpy(dataBuffer, "+-------+-----------+------------+---+------+", 256);
                    send(client_socket, dataBuffer, sizeof(dataBuffer), 0);
                    dataBuffer[256] = '\0';
                }
                else if (strcmp(tokenBuffer[1], "fname\n") == 0)
                {

                    tempList = get(2);

                    strncpy(dataBuffer, "| SID   | Lname     | Fname      | M | GPA  |", 256);
                    send(client_socket, dataBuffer, sizeof(dataBuffer), 0);
                    dataBuffer[256] = '\0';
                    strncpy(dataBuffer, "+-------+-----------+------------+---+------+", 256);
                    send(client_socket, dataBuffer, sizeof(dataBuffer), 0);
                    dataBuffer[256] = '\0';
                    for (i = 0; i < num_students; i++)
                    {
                        sprintf(dataBuffer, "| %-5ld | %-9s | %-9s  | %c | %.2f |", tempList[i].SID, tempList[i].lname, tempList[i].fname, tempList[i].initial, tempList[i].GPA);
                        send(client_socket, dataBuffer, sizeof(dataBuffer), 0);
                        dataBuffer[256] = '\0';
                    }
                    strncpy(dataBuffer, "+-------+-----------+------------+---+------+", 256);
                    send(client_socket, dataBuffer, sizeof(dataBuffer), 0);
                    dataBuffer[256] = '\0';
                }
                else if (strcmp(tokenBuffer[1], "SID\n") == 0)
                {

                    tempList = get(3);

                    strncpy(dataBuffer, "| SID   | Lname     | Fname      | M | GPA  |", 256);
                    send(client_socket, dataBuffer, sizeof(dataBuffer), 0);
                    dataBuffer[256] = '\0';
                    strncpy(dataBuffer, "+-------+-----------+------------+---+------+", 256);
                    send(client_socket, dataBuffer, sizeof(dataBuffer), 0);
                    dataBuffer[256] = '\0';
                    for (i = 0; i < num_students; i++)
                    {
                        sprintf(dataBuffer, "| %-5ld | %-9s | %-9s  | %c | %.2f |", tempList[i].SID, tempList[i].lname, tempList[i].fname, tempList[i].initial, tempList[i].GPA);
                        send(client_socket, dataBuffer, sizeof(dataBuffer), 0);
                        dataBuffer[256] = '\0';
                    }
                    strncpy(dataBuffer, "+-------+-----------+------------+---+------+", 256);
                    send(client_socket, dataBuffer, sizeof(dataBuffer), 0);
                    dataBuffer[256] = '\0';
                }
                else if (strcmp(tokenBuffer[1], "GPA\n") == 0)
                {

                    tempList = get(4);

                    strncpy(dataBuffer, "| SID   | Lname     | Fname      | M | GPA  |", 256);
                    send(client_socket, dataBuffer, sizeof(dataBuffer), 0);
                    dataBuffer[256] = '\0';
                    strncpy(dataBuffer, "+-------+-----------+------------+---+------+", 256);
                    send(client_socket, dataBuffer, sizeof(dataBuffer), 0);
                    dataBuffer[256] = '\0';
                    for (i = 0; i < num_students; i++)
                    {
                        sprintf(dataBuffer, "| %-5ld | %-9s | %-9s  | %c | %.2f |", tempList[i].SID, tempList[i].lname, tempList[i].fname, tempList[i].initial, tempList[i].GPA);
                        send(client_socket, dataBuffer, sizeof(dataBuffer), 0);
                        dataBuffer[256] = '\0';
                    }
                    strncpy(dataBuffer, "+-------+-----------+------------+---+------+", 256);
                    send(client_socket, dataBuffer, sizeof(dataBuffer), 0);
                    dataBuffer[256] = '\0';
                }
            }
            else if (strcmp(tokenBuffer[0], "delete") == 0)
            {

                deleteSID = strtol(tokenBuffer[1], NULL, 10);

                delete (deleteSID);
            }
            else
            {
                send(client_socket, invalidInput, sizeof(invalidInput), 0);
            }

            send(client_socket, secret, sizeof(secret), 0);
        }

        close(client_socket);
    }

    return 0;
}

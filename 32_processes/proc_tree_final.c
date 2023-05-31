#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#define MAX_CHILDS 3

struct job
{
    pid_t jid;
    struct job* j_parent;
    struct job* j_childs[MAX_CHILDS];
};

void nop()
{ return; }

void create_jobs(struct job* childs[MAX_CHILDS], int depth)
{
    for(int i = 0; i < MAX_CHILDS; i++)
    {		
        if(childs[i] == NULL) continue;

        childs[i]->jid = fork();
        switch(childs[i]->jid){
            case -1:
                perror("fork");
            case 0:
                create_jobs(childs[i]->j_childs, depth++);
                sleep(5);
				kill(getpid(),9);
            break;
            default:
                nop(); // wait(NULL);
            break;
        }
    }
}

void link_parent_child(struct job* parent, struct job* child)
{
    child->j_parent = parent; // Связываем родителя с указанием на родителя
    for(int i = 0; i < MAX_CHILDS; i++)
        if(parent->j_childs[i] == NULL) // Если дошли до последнего узла (его потомок 			== NULL)
        {
            parent->j_childs[i] = child; // Этому узлу присвоили ссылку на переданный 				child
            break;
        }
}

void get_childs(struct job* parent)
{
    for(int i = 0; i < MAX_CHILDS; i++)
        if(parent->j_childs[i] != NULL) // Пока не дошли до нижнего узла
        {
            printf("C:%p\n", parent->j_childs[i]);
        }
}

int main()
{
    // Заполняем экземпляры структур
    struct job root = { 0, NULL };
    struct job p1 = {  0, NULL };
    struct job p2 = {  0, NULL };
    struct job p3 = {  0, NULL };
    struct job p4 = {  0, NULL };
    struct job p5 = {  0, NULL };
    
    // Линкуем дерево
    link_parent_child(&root, &p1);
    link_parent_child(&root, &p2);
    
    link_parent_child(&p1, &p3);
    link_parent_child(&p1, &p4);
    
    link_parent_child(&p2, &p5);
    
    // Выводим адреса детей
    printf("root childs:\n"); get_childs(&root);
    printf("p1 childs:\n"); get_childs(&p1);
    printf("p2 childs:\n"); get_childs(&p2);
    printf("p3 childs:\n"); get_childs(&p3);
    printf("p4 childs:\n"); get_childs(&p4);
    printf("p5 childs:\n"); get_childs(&p5);
    
    create_jobs(root.j_childs, 0);

    char mychar[500];
    sprintf(mychar, "pstree -U -h -p %d", getpid());
    system(mychar);
    
    if (wait(NULL) == -1)
		perror("wait 1");
	if (wait(NULL) == -1)
		perror("wait 2");
    
    return 0;
}

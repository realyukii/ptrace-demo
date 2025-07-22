#include <stdlib.h>
#include <stdio.h>

struct item {
	int nr;
	struct item *next;
};

struct queue {
	struct item *head;
	struct item *tail;
};

static struct item *init_item(struct queue *q)
{
	struct item *i = malloc(sizeof(*i));
	i->next = NULL;
	return i;
}

static void enqueue_item(struct queue *q, struct item *i)
{
	if (q->head) {
		q->tail->next = i;
		q->tail = i;
	} else
		q->head = q->tail = i;
}

static void dequeue_item(struct queue *q)
{
	struct item *i = q->head;
	if (!i)
		return;

	q->head = i->next;
	if (!q->head)
		q->tail = NULL;

	free(i);
}

int main(void)
{
	static const char msg[] = "enter a number (zero to dequeue): ";
	static char buffer[255];
	struct queue q;
	struct item *i;
	struct item *list;
	int nr, counter;

	counter = 1;
	printf(msg);
	q.tail = q.head = NULL;
	while (fgets(buffer, sizeof(buffer), stdin)) {
		nr = strtol(buffer, NULL, 10);
		i = init_item(&q);
		if (nr) {
			i->nr = nr;
			enqueue_item(&q, i);
		} else
			dequeue_item(&q);

		printf("\e[1;1H\e[2J");
		list = q.head;
		if (!list)
			printf("the queue is empty.\n");
		else
			while (list) {
				printf("%d. %d\n", counter, list->nr);
				list = list->next;
				counter++;
			}

		counter = 1;

		printf("");
		printf(msg);
		fflush(stdout);
	}
	
	return 0;
}
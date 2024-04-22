/*
 * socket demonstrations:
 * This is the server side of an "internet domain" socket connection, for
 * communicating over the network.
 *
 * In this case we are willing to wait for chatter from the client
 * _or_ for a new connection.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#ifndef PORT
    #define PORT 52349
#endif

# define SECONDS 10
# define MAX_BUF 128
# define IMG_HEIGHT 36

const char mon_lines[IMG_HEIGHT][MAX_BUF] = {
        "                      ,-'`\\\r\n",
    "                  _,\"'    j\r\n",
    "           __....+       /               .\r\n",
    "       ,-'\"             /               , `-._.'.\r\n",
    "      /                (              ,'       .'\r\n",
    "     |            _.    \\             \\   ---._ `-.\r\n",
    "     ,|    ,   _.'  Y    \\             `- ,'   \\   `.`.\r\n",
    "     l'    \\ ,'._,\\ `.    .              /       ,--. l\r\n",
    "  .,-        `._  |  |    |              \\       _   l .\r\n",
    " /              `\"--'    /              .'       ``. |  \r\n",
    ".\\    ,                 |                .        \\ `. '\r\n",
    "`.                .     |                '._  __   ,. \\'\r\n",
    "  `-..--------...'       \\                  `'  `-\"'.  \\\r\n",
    "      `......___          `._                        |  \\\r\n",
    "               /`            `..                     |   .\r\n",
    "              /|                `-.                  |    L\r\n",
    "             / |               \\   `._               .    |\r\n",
    "           ,'  |,-\"-.   .       .     `.            /     |\r\n",
    "         ,'    |     '   \\      |       `.         /      |\r\n",
    "       ,'     /|       \\  .     |         .       /       |\r\n",
    "     ,'      / |        \\  .    +          \\    ,'       .'\r\n",
    "    .       .  |         \\ |     \\          \\_,'        / j\r\n",
    "    |       |  L          `|      .          `        ,' '\r\n",
    "    |    _. |   \\          /      |           .     .' ,'\r\n",
    "    |   /  `|    \\        .       |  /        |   ,' .'\r\n",
    "    |   ,-..\\     -.     ,        | /         |,.' ,'\r\n",
    "    `. |___,`    /  `.   /`.       '          |  .'\r\n",
    "      '-`-'     j     ` /.\"7-..../|          ,`-'\r\n",
    "                |        .'  / _/_|          .\r\n",
    "                `,       `\"'/\"'    \\          `.\r\n",
    "                  `,       '.       `.         |\r\n",
    "             __,.-'         `.        \\'       |\r\n",
    "            /_,-'\\          ,'        |        _.\r\n",
    "             |___.---.   ,-'        .-':,-\"`\\,' .\r\n",
    "                  L,.--\"'           '-' |  ,' `-.\\\r\n",
    "                                        `.' \r\n"
    };

const char mon_attacked_lines[IMG_HEIGHT][MAX_BUF] = {
        "                      ,-'`\\\r\n",
    "                  _,\"'    j\r\n",
    "           __....+       /  X             .\r\n",
    "       ,-'\"                X       , `-._.'.\r\n",
    "      /                (   XX              ,'       .'\r\n",
    "     |            _.     X X          \\   ---._ `-.\r\n",
    "     ,|    ,   _.'  Y   X X      `- ,'   \\   `.`.\r\n",
    "     l'    \\ ,'._,\\  X.X              /       ,--. l\r\n",
    "  .,-        `._  |  |X X        |              \\       _   l .\r\n",
    " /              `\"--X/X              .'       ``. |  \r\n",
    ".\\    ,            X X                    .        \\ `. '\r\n",
    "`.                .X X                    '._  __   ,. \\'\r\n",
    "  `-..--------...' X      \\                  `'  `-\"'.  \\\r\n",
    "      `......___X  X        `._                        |  \\\r\n",
    "             X X X/`          `..                     |   .\r\n",
    "           XXX   /|             `-.                  |    L\r\n",
    "         XXXX    /             \\   `._               .    |\r\n",
    "           ,'  |,-\"-.   .       .     `.            /     |\r\n",
    "         ,'    |     '   \\      |       `.         /      |\r\n",
    "       ,'     /|       \\  .     |         .       /       |\r\n",
    "     ,'      / |        \\  .    +          \\    ,'       .'\r\n",
    "    .       .  |         \\ |     \\          \\_,'        / j\r\n",
    "    |       |  L          `|      .          `        ,' '\r\n",
    "    |    _. |   \\          /      |           .     .' ,'\r\n",
    "    |   /  `|    \\        .       |  /        |   ,' .'\r\n",
    "    |   ,-..\\     -.     ,        | /         |,.' ,'\r\n",
    "    `. |___,`    /  `.   /`.       '          |  .'\r\n",
    "      '-`-'     j     ` /.\"7-..../|          ,`-'\r\n",
    "                |        .'  / _/_|          .\r\n",
    "                `,       `\"'/\"'    \\          `.\r\n",
    "                  `,       '.       `.         |\r\n",
    "             __,.-'         `.        \\'       |\r\n",
    "            /_,-'\\          ,'        |        _.\r\n",
    "             |___.---.   ,-'        .-':,-\"`\\,' .\r\n",
    "                  L,.--\"'           '-' |  ,' `-.\\\r\n",
    "                                        `.' \r\n"
    };

struct client {
    int fd;
    struct in_addr ipaddr;
    struct client *next;
    char *name; // max 15
    char *msg; // max MAX_BUF
    struct client *lastmatch;
    struct battlestats *stats;
    unsigned int hasname;
    unsigned int inmatch;
    unsigned int namelen; // [0, 15]
    unsigned int msglen; // [0, MAX_BUF-1]
};

struct battlestats {
    int hp;
    unsigned int numpm;
    unsigned int sendingmsg;
    unsigned int turn;
};

static struct client *addclient(struct client *top, int fd, struct in_addr addr);

static struct client *removeclient(struct client *top, int fd);

static void broadcast(struct client *top, char *s, int size, struct client *self);

int handleclient(struct client *p, struct client *top);

//Battle-specific functions 

struct client *findmatch(struct client *head, char **name1, char **name2);
//two players that just fought together aren't immediately matched together again
//start at the head client, iterate thru all other clients
//last battle stored in variable (plr1, plr2); make sure this doesn't reappear

void startbattle(struct client *p1, struct client *p2);

int decidefirststrike();

void sendmessage(char *msg, struct client *p);

int attack(struct client *poff, struct client *pdef);

void powermove(struct client *poff, struct client *pdef);

void displaybattleinfo(struct client *p);

void handlewinner(struct client *top, struct client *p1, struct client *p2);

void initnumgenerator(void);

void cycleturn(struct client *top, struct client *p);

void sethp(struct client *p);

void setnumpm(struct client *p);

int checkifwon(struct client *p1, struct client *p2);

int bindandlisten(void);

int main(void) {
    initnumgenerator();
    int clientfd, maxfd, nready;
    struct client *p;
    struct client *head = NULL; // is a linked list
    socklen_t len;
    struct sockaddr_in q;  // will hold the connecting client's information 
    // socket address internet (has 4 attributes: sin_family = AF_INET, 
    // sin_port = htons(int portnum), sin_addr.s_addr = INADDR_ANY, sin_zero[8])
    struct timeval tv;
    fd_set allset;
    fd_set rset;

    int i;

    // before passing to accept, should be of the same family as set up for client
    // in the next bindandlisten() call
    q.sin_family = AF_INET;

    int listenfd = bindandlisten();
    // initialize allset and add listenfd to the
    // set of file descriptors passed into select
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    // maxfd identifies how far into the set to search
    maxfd = listenfd;

    while (1) {
        // make a copy of the set before we pass it into select
        rset = allset;
        /* timeout in seconds (You may not need to use a timeout for
        * your assignment)*/
        tv.tv_sec = SECONDS;
        tv.tv_usec = 0;  /* and microseconds */

        // with timeout
        nready = select(maxfd + 1, &rset, NULL, NULL, &tv);  // change final param to NULL to remove timeout

        if (nready == 0) {
            // printf("No response from clients in %d seconds\n", SECONDS);
            continue;
        }

        if (nready == -1) {
            perror("select");
            continue;
        }
        // nready > 0 (new client is connecting)
        if (FD_ISSET(listenfd, &rset)){
            printf("a new client is connecting\n");
            len = sizeof(q);
            if ((clientfd = accept(listenfd, (struct sockaddr *)&q, &len)) < 0) {
                perror("accept");
                exit(1);
            }
            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd; // maxfd is the largest fd number in allset
            }
            printf("connection from %s\n", inet_ntoa(q.sin_addr));
		
	        // add the client to the list of clients
            head = addclient(head, clientfd, q.sin_addr); // asks the client for their name
        }

        for(i = 0; i <= maxfd; i++) {
            if (FD_ISSET(i, &rset)) { // if fd i has something to say
                for (p = head; p != NULL; p = p->next) { // find corresponding client struct
                    if (p->fd == i) {
                        int result = handleclient(p, head);
                        if (result == -1) {
                            int tmp_fd = p->fd;
                            head = removeclient(head, p->fd);
                            FD_CLR(tmp_fd, &allset);
                            close(tmp_fd);
                        }
                        break;
                    }
                }
            }
        }
    }
    return 0;
}

/*
* Reads inputs from the client p (could either be a name declaration, a 
* command input, a (s)poken message, or ignored);
* Finds a match for the client once the client has a name.
*/
int handleclient(struct client *p, struct client *top) {
    char buf[2];
    char outbuf[512];
    int len = read(p->fd, buf, 1);

    //if the player doesn't yet have a name, set their name to the next full line they type
    if(p->hasname == 0) {
	    // end of the name has been reached (max at 15 characters)
        if(buf[0] == '\n' || p->namelen == 15) {
            
            // cosmetic newline
            if (p->namelen == 15) {
                if (write(p->fd, "\r\n", 2) < 2) {
                    printf("Newline after long name input could not write");
                    perror("write");
                    exit(1);
                }  
            }
            
            p->hasname = 1;
            p->name[p->namelen] = '\0';

            // print welcome message
            snprintf(outbuf, MAX_BUF, "Welcome, %s! ", p->name);
            if(write(p->fd, outbuf, strnlen(outbuf, MAX_BUF)) < strnlen(outbuf, MAX_BUF)) {
                printf("Client (%d) %s did not receive message.\r\n", p->fd, inet_ntoa(p->ipaddr));
		        perror("write");
		        exit(1);
            }
            
            // broadcast **[name] enters the arena** to everyone exept [name]
            char arenamsg[MAX_BUF];
            snprintf(arenamsg, (sizeof(p->name)+26), "**%s enters the arena**\r\n", p->name);
            broadcast(top, arenamsg, strnlen(outbuf, MAX_BUF), p);

            // print awaiting opponent message
            char waitprompt[] = "Awaiting Opponent...\r\n";
            if(write(p->fd, waitprompt, 23) < 23) {
                printf("Client (%d) %s did not receive message.\r\n", p->fd, inet_ntoa(p->ipaddr));
		        perror("write");
	            exit(1);
            }
	    
        //append the latest character to their name
        } else {
            (p->name)[p->namelen] = buf[0];
            p->namelen++;
        }
    
    // The player fd has something to say
    } else if (len > 0) {
        
        // check if player is in match and it is their turn
        if(p->inmatch == 1 && p->stats->turn == 1) {
           
           // if not in the process of sending a message, check for command: a, p, or s
            if(p->stats->sendingmsg == 0) {
                if(buf[0] == 'a') {
                    attack(p, p->lastmatch);
                    cycleturn(top, p);
                    
                } else if(buf[0] == 'p' && p->stats->numpm > 0) {
                    powermove(p, p->lastmatch);
                    cycleturn(top, p);
                    
                } else if(buf[0] == 's') {
                    p->stats->sendingmsg = 1;
                    if (write(p->fd, "\r\nSpeak: ", 9) < 9) {
                        printf("Newline after a/p/s command could not write");
                        perror("write");
                        exit(1);
                    }
                }
            } else {
                // player is sending a message (pressed s command earlier)
                // send the player's message if it's compelete

                // to avoid overflow in <p->msg>, check that msg length is 
                // below MAX_BUF - 1 (excluding null terminator)
                if (buf[0] == '\n' || p->msglen == MAX_BUF - 1) {
                    
                    if (p->msglen == MAX_BUF - 1) {
                        if (write(p->fd, "\r\n", 2) < 2) {
                            printf("Newline after long message could not write");
                            perror("write");
                            exit(1);
                        } 
                    }
                    
                    (p->msg)[p->msglen] = '\0';
                    sendmessage(p->msg, (struct client *)p->lastmatch);
                    
                    // clear the message
                    memset(p->msg, 0, strlen(p->msg));
                    p->msglen = 0;

                    p->stats->sendingmsg = 0; //set to default state of not in progress of sending message
                }
                //add to the player's message if it's incomplete
                else { 
                    (p->msg)[p->msglen] = buf[0];
                    p->msglen++;
                }
            }
        }
        return 0;
    } else if (len <= 0) {
        // socket is closed
        printf("Disconnect from %s\n", inet_ntoa(p->ipaddr));
        
        // handle disconnect (check no garbage in front of awaiting message after it)
        if (p->inmatch && p->lastmatch->inmatch) {
            char msg[46]; // 44 = max sizeof msg below
            snprintf(msg, (sizeof(p->name) + 29), "\r\n--%s dropped. You win!\r\n\r\n", p->name);
            if (write(p->lastmatch->fd, msg, strnlen(msg, 46)) < strnlen(msg,46)) {
                printf("Drop message could not be written.");
                perror("write");
                exit(1);
            }

            // TODO: put p->lastmatch back into queue -- see findmatch function
            p->lastmatch->inmatch = 0;

            char waitprompt[] = "Awaiting Opponent...\r\n";
            if(write(p->lastmatch->fd, waitprompt, 23) < 23) {
                printf("Client (%d) %s did not receive message.\r\n", p->lastmatch->fd, inet_ntoa(p->lastmatch->ipaddr));
		        perror("write");
	            exit(1);
            }

            //find a new opponent
            char *name1;
            char *name2;
            struct client *matchedplr = findmatch(top, &name1, &name2);
            if (matchedplr != NULL) {
                //clients have been matched
                printf("%s has been matched with %s\r\n", name1, name2);
                startbattle(matchedplr, matchedplr->lastmatch);
            } else {
                //clients have not been matched
                printf("players has not been matched yet\r\n");
            }
        }
        return -1;
    } 

    //if the player is not in a match yet, find a match for them to join
    if (p->hasname == 1 && p->inmatch == 0) {
        char *name1;
        char *name2;
        
        memset(outbuf, 0, sizeof(outbuf));

        //what findmatch does: success: modifty the inmatch and lastmatch attributes
        //of two players. failure: do nothing
        struct client *matchedplr = findmatch(top, &name1, &name2);
	    if (matchedplr != NULL) {
            //clients have been matched
            printf("%s has been matched with %s\r\n", name1, name2);
            startbattle(matchedplr, matchedplr->lastmatch);
        } else {
            //clients have not been matched
            printf("players have not been matched yet\r\n");
        }
    }

    // return 0 to indicate end of function 
    return 0;
}

 /* bind and listen, abort on error
  * returns FD of listening socket
  */
int bindandlisten(void) {
    struct sockaddr_in r;
    int listenfd;  // stream socket

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    int yes = 1;
    if ((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) == -1) {
        perror("setsockopt");
    }

    // memset call sets all entries to 0
    memset(&r, '\0', sizeof(r));
    r.sin_family = AF_INET;
    r.sin_addr.s_addr = INADDR_ANY;
    r.sin_port = htons(PORT);

    // sets up and sets address for stream socket
    if (bind(listenfd, (struct sockaddr *)&r, sizeof r) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(listenfd, 5) < 0) {
        perror("listen");
        exit(1);
    }
    return listenfd;
}

static struct client *addclient(struct client *top, int fd, struct in_addr addr) {
    char *msg= "What is your name?";

    struct client *p = malloc(sizeof(struct client));
    if (!p) {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));
    if (write(fd, msg, strnlen(msg, MAX_BUF)) == -1) {
        perror("write");
	    exit(1);
    }
    
    p->stats = malloc(sizeof(struct battlestats));
    p->fd = fd;
    p->ipaddr = addr;
    p->next = top;
    p->lastmatch = NULL;
    p->hasname = 0;
    p->inmatch = 0;
    p->name = malloc(16); // name can be up to 15 characters long
    (p->name)[0] = '\0'; //so strcat works later
    p->msg = malloc(MAX_BUF);

    p->namelen = 0;
    p->msglen = 0;

    top = p;
    return top;
}

static struct client *removeclient(struct client *top, int fd) {
    struct client **p;

    for (p = &top; *p && (*p)->fd != fd; p = &(*p)->next)
        ;
    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {
        struct client *t = (*p)->next;
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));

        // remove the player from their match, if they're in one
        if ((*p)->inmatch == 1) {
            struct client *opponent = (*p)->lastmatch;
            opponent->inmatch = 0;
        }

        // broadcast that they left
        char outbuf[MAX_BUF];
        snprintf(outbuf, MAX_BUF, "**%s leaves**\r\n", (*p)->name);
        broadcast(top, outbuf, strnlen(outbuf, MAX_BUF), NULL);

        // free malloc'd attributes before freeing the client itself
        free((*p)->name);
        free((*p)->msg);
        free((*p)->stats);

        free(*p);
        *p = t;
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n",
                 fd);
    }
    return top;
}


static void broadcast(struct client *top, char *s, int size, struct client *self) {
    struct client *p;
    for (p = top; p; p = p->next) {
        if ((self) && (p->ipaddr.s_addr == self->ipaddr.s_addr)) {
            continue; // option to not broadcast to the one joining/leaving the arena, themselves
        }
        else if (write(p->fd, s, size) == -1) {
            removeclient(p, p->fd);
            perror("write");
        };
    }
    /* should probably check write() return value and perhaps remove client -- done? */
}

struct client *findmatch(struct client *head, char **name1, char **name2) {
    //p1 and p2 have name set to NULL

    //start at the head client, iterate thru all other clients
    struct client *curr = head;
    struct client *p1 = NULL;
    struct client *p2 = NULL;
    //stop searching when all clients have been searched
    while ((p1 == NULL || p2 == NULL) && curr != NULL) {
        //if this player is new, they're eligible for a new match with anybody
        if (curr->hasname == 1 && curr->inmatch == 0) {
            if (p1 == NULL) {
                p1 = curr;
                p1->inmatch = 1;
            } else if (p2 == NULL) {
                //make sure this wasn't the last match that happened
                if (p1->lastmatch != curr) {
                    p2 = curr;
                    p2->inmatch = 1;
                } else {
                    printf("that match already happened!\n");
                }
            }
        }

        curr = curr->next;
    }

    //in the case of success, p1 and p2 will be the matched clients, else return NULL
    if (p1 != NULL && p2 != NULL) {
        p1->lastmatch = p2;
        p2->lastmatch = p1;
        *name1 = p1->name;
        *name2 = p2->name;
        return p1;
    } else {
        //Reset the attributes of both players back to what they were, if needed
        if (p1 != NULL) {
            p1->inmatch = 0;
        }
        if (p2 != NULL) {
            p2->inmatch = 0;
        }
        return NULL;
    }
}

/*
 * Initializes the number generator using the time of 
 * day as the seed. This allows for variation of the 
 * pseudo-random number for every program call. 
 */
void initnumgenerator() {
    srand((unsigned) time(NULL));
}

/*
 * Will return an int, either 1 or 2, indicating
 * which player will be given the first move (i.e 
 * the player who was in queue first will be labelled
 * 1, and the player who was matched will be labelled 
 * 2).
 */
int decidefirststrike() {
    return ((rand() % 2) + 1);
}

/*
 * Writes str <msg> to the fds of the clients in this battle, when p uses the (s) command. 
 */
void sendmessage(char *msg, struct client *p) {
    char sender_msg[MAX_BUF];
    char receiver_msg[MAX_BUF];
    snprintf(sender_msg, MAX_BUF, "You speak: %s\r\n\r\n", msg);
    snprintf(receiver_msg, MAX_BUF, "%s takes a break to tell you:\n%s\r\n\r\n", p->lastmatch->name, msg);
    sender_msg[MAX_BUF - 2] = '\n';
    sender_msg[MAX_BUF - 1] = '\n';
    receiver_msg[MAX_BUF - 2] = '\n';
    receiver_msg[MAX_BUF - 1] = '\n';
    if(write(p->lastmatch->fd, sender_msg, strnlen(sender_msg, MAX_BUF)) < strnlen(sender_msg, MAX_BUF)) {
        printf("Failed to send message to fd (%d)\n", p->fd);
	    perror("write");
	    exit(1);
    }
    if(write(p->fd, receiver_msg, strnlen(receiver_msg, MAX_BUF)) < strnlen(receiver_msg, MAX_BUF)) {
        printf("Failed to send message to fd (%d)\n", p->lastmatch->fd);
	    perror("write");
	    exit(1);
    }
    displaybattleinfo(p);
}

void showmonimg(struct client *off, struct client *def, int attacked) {
    char offmsg[MAX_BUF];
    snprintf(offmsg, MAX_BUF, "\r\n=========================\r\n======= This is %s =======\r\n=========================\r\n", def->name);
    write(off->fd, offmsg, strnlen(offmsg, MAX_BUF));

    for (int i = 0; i < IMG_HEIGHT; i++) {
	if (attacked == 0) {
            snprintf(offmsg, MAX_BUF, "%s", mon_lines[i]);
	} else {
	    snprintf(offmsg, MAX_BUF, "%s", mon_attacked_lines[i]);
	}
        write(off->fd, offmsg, strnlen(offmsg, MAX_BUF));
    }
}

/*
 * Calculates and sets the <stats.hp> field of the client
 * <*p> after being hit with attack of <amt> damage. <amt> is 
 * determined using the random number generator, with lowerbound
 * 2 and upperbound 6 (inclusive). Prints messages to the offensive
 * player <*poff> and the defensive player <*pdef> which indicate how 
 * much they hit <*pdef> for and how much they were hit for by <*poff>,
 * respectively. 
 * 
 * Returns an int indicating the remaining hitpoints of client <*pdef>.
 */
int attack(struct client *poff, struct client *pdef) {
    int amt = (rand() % 5) + 2;
    pdef->stats->hp -= amt;
    char offmsg[MAX_BUF];
    char defmsg[MAX_BUF];

    //show monster image to attacker
    showmonimg(poff, pdef, 1);

    snprintf(offmsg, MAX_BUF, "\r\nYou hit %s for %d damage!\r\n", pdef->name, amt);
    snprintf(defmsg, MAX_BUF, "%s hits you for %d damage!\r\n", poff->name, amt);

    if(write(poff->fd, offmsg, strnlen(offmsg, MAX_BUF)) < strnlen(offmsg, MAX_BUF)) {
        printf("Client (%d) %s did not receive message.\r\n", poff->fd, inet_ntoa(poff->ipaddr));
	    perror("write");
	    exit(1);
    }
    if(write(pdef->fd, defmsg, strnlen(defmsg, MAX_BUF)) < strnlen(defmsg, MAX_BUF)) {
        printf("Client (%d) %s did not receive message.\r\n", pdef->fd, inet_ntoa(pdef->ipaddr));
	    perror("write");
	    exit(1);
    }

    return pdef->stats->hp;
}

/*
    poff tries to use powermove on opponent, pdef
    returns 1 if a powermove has been used by poff (regardless of hitting or missing)
*/
void powermove(struct client *poff, struct client *pdef) {
    if (poff->stats->numpm > 0) {   
        int pwr_attack = 3 * ((rand() % 5) + 2);
        int ran = rand();
        char offmsg[MAX_BUF]; 
        char defmsg[MAX_BUF];

	//show monster image to attacker
	showmonimg(poff, pdef, 1);

        if (ran % 2 == 0) {
            pdef->stats->hp -= pwr_attack; // did not miss: reduces hp of pdef
            snprintf(offmsg, MAX_BUF, "\r\nYou hit %s for %d damage!\r\n", pdef->name, pwr_attack);
            snprintf(defmsg, MAX_BUF, "%s powermoves you for %d damage!\r\n", poff->name, pwr_attack);
            if(write(poff->fd, offmsg, strnlen(offmsg, MAX_BUF)) < strnlen(offmsg, MAX_BUF)) {
                printf("Client (%d) %s did not receive message.", poff->fd, inet_ntoa(poff->ipaddr));
		        perror("write");
		        exit(1);
            }
            if(write(pdef->fd, defmsg, strnlen(defmsg, MAX_BUF)) < strnlen(defmsg, MAX_BUF)) {
                printf("Client (%d) %s did not receive message.", pdef->fd, inet_ntoa(pdef->ipaddr));
		        perror("write");
		        exit(1);
            }
        } else { // missed: does not reduce hp of pdef
            snprintf(offmsg, MAX_BUF, "\r\nYou missed!\r\n");
            snprintf(defmsg, MAX_BUF, "%s missed you!\r\n", poff->name);
            if(write(poff->fd, offmsg, strnlen(offmsg, MAX_BUF)) < strnlen(offmsg, MAX_BUF)) {
                printf("Client (%d) %s did not receive message.", poff->fd, inet_ntoa(poff->ipaddr));
		        perror("write");
		        exit(1);
            }
            if(write(pdef->fd, defmsg, strnlen(defmsg, MAX_BUF)) < strnlen(defmsg, MAX_BUF)) {
                printf("Client (%d) %s did not receive message.", pdef->fd, inet_ntoa(pdef->ipaddr));
		        perror("write");
		        exit(1);
            }
        }
	    // consume one powermove
        poff->stats->numpm -= 1;
	    return;
    }
    return;
}


/*
    prints stats (health, attack) to player p (should be used each time p, or their opponent, has a turn,
    and after each message sent using the s command)
*/
void displaybattleinfo(struct client *p) {
    char offmsg[MAX_BUF];
    char defmsg[MAX_BUF];
    struct client *off, *def;
    if(p->stats->turn == 1) {
        off = p;
        def = p->lastmatch;
    } else {
        off = p->lastmatch;
        def = p;
    }
    //display monster image to offender
    showmonimg(off, def, 0);

    // no powermove option in display if offense player's numpm < 0
    if (off->stats->numpm > 0) {
        snprintf(offmsg, MAX_BUF, "Your hitpoints: %d\r\nYour powermoves: %d\r\n\r\n%s's hitpoints: %d\r\n\r\n(a)ttack\r\n(p)owermove\r\n(s)peak something\r\n", 
            off->stats->hp, off->stats->numpm, def->name, def->stats->hp);
    } else {
        snprintf(offmsg, MAX_BUF, "Your hitpoints: %d\r\nYour powermoves: %d\r\n\r\n%s's hitpoints: %d\r\n\r\n(a)ttack\r\n(s)peak something\r\n", 
            off->stats->hp, off->stats->numpm, def->name, def->stats->hp); 
    }
    snprintf(defmsg, MAX_BUF, "Your hitpoints: %d\r\nYour powermoves: %d\r\n\r\n%s's hitpoints: %d\r\nWaiting for %s to strike...\r\n", 
        def->stats->hp, def->stats->numpm, off->name, off->stats->hp, off->name);

    if(write(off->fd, offmsg, strnlen(offmsg, MAX_BUF)) < strnlen(offmsg, MAX_BUF)) {
        printf("Client (%d) %s did not receive message.", off->fd, inet_ntoa(off->ipaddr));
	    perror("write");
	    exit(1);
    }
    if(write(def->fd, defmsg, strnlen(defmsg, MAX_BUF)) < strnlen(defmsg, MAX_BUF)) {
        printf("Client (%d) %s did not receive message.", def->fd, inet_ntoa(def->ipaddr));
	    perror("write");
	    exit(1);
    }
};

/*
    handle a battle when a winner has been decided (as p1): put back into waiting room
*/
void handlewinner(struct client *top, struct client *p1, struct client *p2) {
    char p1msg[MAX_BUF];
    char p2msg[MAX_BUF];
    snprintf(p1msg, MAX_BUF, "%s gives up. You win!\r\n", p2->name);
    snprintf(p2msg, MAX_BUF, "You are no match for %s. You scurry away...\r\n", p1->name);
    if(write(p1->fd, p1msg, strnlen(p1msg, MAX_BUF)) < strnlen(p1msg, MAX_BUF)) {
        printf("Client (%d) %s did not receive message.", p1->fd, inet_ntoa(p1->ipaddr));
	    perror("write");
	    exit(1);
    }
    if(write(p2->fd, p2msg, strnlen(p2msg, MAX_BUF)) < strnlen(p2msg, MAX_BUF)) {
        printf("Client (%d) %s did not receive message.", p2->fd, inet_ntoa(p2->ipaddr));
	    perror("write");
	    exit(1);
    }

    // TODO: end battle -- put back into waitlist, don't displaybattleinfo following this a/p command
    p1->inmatch = 0;
    p2->inmatch = 0;

    // print awaiting opponent message
    char waitprompt[] = "Awaiting Opponent...\r\n";
    if(write(p1->fd, waitprompt, 23) < 23) {
        printf("Client (%d) %s did not receive message.\r\n", p1->fd, inet_ntoa(p1->ipaddr));
		perror("write");
	    exit(1);
    }
    if(write(p2->fd, waitprompt, 23) < 23) {
         printf("Client (%d) %s did not receive message.\r\n", p2->fd, inet_ntoa(p2->ipaddr));
		perror("write");
	    exit(1);
    }

    //start as many new matches as there are unmatched players
    for (int i = 0; i < 2; i++) {
        char *name1;
        char *name2;
        struct client *matchedplr = findmatch(top, &name1, &name2);
        if (matchedplr != NULL) {
            //clients have been matched
            printf("%s has been matched with %s\r\n", name1, name2);
            startbattle(matchedplr, matchedplr->lastmatch);
        } else {
            //clients have not been matched
             printf("clients have not been matched yet\r\n");
	}
    }
};

//return 1 if player 1 has won, and 2 if player 2 has won, else return 0 if nobody wins
int checkifwon(struct client *p1, struct client *p2) {
    int hp1 = p1->stats->hp;
    int hp2 = p2->stats->hp;
    if (hp1 <= 0) {
	    return 2;
    } else if (hp2 <= 0) {
	    return 1;
    }
    return 0;
}

/*
 * Starts a battle between two players <p1> and <p2>. 
 * Initializes both players hp and number of powermoves.
 * The order in which players will move is also established.
 */
void startbattle(struct client *p1, struct client *p2) {
    // display message about engaging in battle with the new opponent
    char p1msg[MAX_BUF];
    char p2msg[MAX_BUF];
    snprintf(p1msg, MAX_BUF, "You engage %s!\r\n", p2->name);
    snprintf(p2msg, MAX_BUF, "You engage %s!\r\n", p1->name);
    if(write(p1->fd, p1msg, strnlen(p1msg, MAX_BUF)) < strnlen(p1msg, MAX_BUF)) {
        printf("Client (%d) %s did not receive message.", p1->fd, inet_ntoa(p1->ipaddr));
	    perror("write");
	    exit(1);
    }
    if(write(p2->fd, p2msg, strnlen(p2msg, MAX_BUF)) < strnlen(p2msg, MAX_BUF)) {
        printf("Client (%d) %s did not receive message.", p2->fd, inet_ntoa(p2->ipaddr));
	    perror("write");
	    exit(1);
    }
    
    // generate and set hp for both players
    sethp(p1);
    sethp(p2);

    // generate and set number of power moves for both players
    setnumpm(p1);
    setnumpm(p2);

    // decide which of the two players moves first and set 
    // the turn attribute for each player
    if(decidefirststrike() == 1) {
        p1->stats->turn = 1;
        p2->stats->turn = 0;
    } else {
        p2->stats->turn = 1;
        p1->stats->turn = 0;
    }

    // set default state of not currently sending message
    p1->stats->sendingmsg = 0;
    p2->stats->sendingmsg = 0;

    // display information to both players
    displaybattleinfo(p1);
}

/*
 * Cycles to the next player's turn in a match.
 * Displays updated information to both players.
 */
void cycleturn(struct client *top, struct client *p) {
    // gather information of opposing player
    struct client *opp = p->lastmatch;
    // cycle turns for both players
    p->stats->turn = 0;
    opp->stats->turn = 1;

    //check if anybody has won. If so, print it
    int result = checkifwon(p, opp);
    if (result == 1) {
	    handlewinner(top, p, opp);
    } else if (result == 2) {
	    handlewinner(top, opp, p);
    } else {
        displaybattleinfo(p); // only display battle info if battle is not over
    }
}

/*
 * Generates and sets hp for player <p>.
 */
void sethp(struct client *p) {
    p->stats->hp = (rand() % 11) + 20;
}

/*
 * Generates and sets the number of powermoves for player <p>.
 */
void setnumpm(struct client *p) {
    p->stats->numpm = (rand() % 3) + 1;
}


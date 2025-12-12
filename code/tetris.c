#include "tetris.h"

static struct sigaction act, oact;

int main()
{
	int exit = 0;

	initscr();
	noecho();
	keypad(stdscr, TRUE);
	srand((unsigned int)time(NULL));
	createRankList();
	while (!exit)
	{
		clear();
		switch (menu())
		{
		case MENU_PLAY:
			play();
			break;
		case MENU_RANK:
			rank();
			break;
		case MENU_REC_PLAY:
			rflag=1;
			recommendedPlay();
			rflag=0;
			break;
		case MENU_EXIT:
			exit = 1;
			break;
		default:
			break;
		}
	}

	endwin();
	system("clear");
	return 0;
}

void InitTetris()
{
	int i, j;

	for (j = 0; j < HEIGHT; j++)
		for (i = 0; i < WIDTH; i++)
			field[j][i] = 0;

	for(int i=0;i<BLOCK_NUM;i++)
	{
		nextBlock[i]=rand()%7;
	}

	blockRotate = 0;
	blockY = -1;
	blockX = WIDTH / 2 - 2;
	score = 0;
	gameOver = 0;
	timed_out = 0;
	recRoot = malloc(sizeof(RecNode));
	recRoot->lv = 0;
	recRoot->score = 0;
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			recRoot->f[i][j] = field[i][j];
		}
	}

	DrawOutline();
	DrawField();
	modified_recommend(recRoot);
	DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
	DrawNextBlock(nextBlock);
	PrintScore(score);
}

void DrawOutline()
{
	int i, j;
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	DrawBox(0, 0, HEIGHT, WIDTH);

	/* next block을 보여주는 공간의 태두리를 그린다.*/
	move(2, WIDTH + 10);
	printw("NEXT BLOCK");
	DrawBox(3, WIDTH + 10, 4, 8);

	DrawBox(9, WIDTH + 10, 4, 8);

	/* score를 보여주는 공간의 태두리를 그린다.*/
	move(15, WIDTH + 10);
	printw("SCORE");
	DrawBox(16, WIDTH + 10, 1, 8);
}

int GetCommand()
{
	int command;
	command = wgetch(stdscr);
	switch (command)
	{
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ': /* space key*/
		/*fall block*/
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command)
{
	int ret = 1;
	int drawFlag = 0;
	switch (command)
	{
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if ((drawFlag = CheckToMove(field, nextBlock[0], (blockRotate + 1) % 4, blockY, blockX)))
			blockRotate = (blockRotate + 1) % 4;
		break;
	case KEY_DOWN:
		if ((drawFlag = CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX)))
			blockY++;
		break;
	case KEY_RIGHT:
		if ((drawFlag = CheckToMove(field, nextBlock[0], blockRotate, blockY, blockX + 1)))
			blockX++;
		break;
	case KEY_LEFT:
		if ((drawFlag = CheckToMove(field, nextBlock[0], blockRotate, blockY, blockX - 1)))
			blockX--;
		break;
	default:
		break;
	}
	if (drawFlag)
		DrawChange(field, command, nextBlock[0], blockRotate, blockY, blockX);
	return ret;
}

void DrawField()
{
	int i, j;
	for (j = 0; j < HEIGHT; j++)
	{
		move(j + 1, 1);
		for (i = 0; i < WIDTH; i++)
		{
			if (field[j][i] == 1)
			{
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else
				printw(".");
		}
	}
}

void PrintScore(int score)
{
	move(17, WIDTH + 11);
	printw("%8d", score);
}

void DrawNextBlock(int *nextBlock)
{
	int i, j;
	for (i = 0; i < 4; i++)
	{
		move(4 + i, WIDTH + 13);
		for (j = 0; j < 4; j++)
		{
			if (block[nextBlock[1]][0][i][j] == 1)
			{
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else
				printw(" ");
		}
	}

	for (i = 0; i < 4; i++)
	{
		move(10 + i, WIDTH + 13);
		for (j = 0; j < 4; j++)
		{
			if (block[nextBlock[2]][0][i][j] == 1)
			{
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else
				printw(" ");
		}
	}
}

void DrawBlock(int y, int x, int blockID, int blockRotate, char tile)
{
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
		{
			if (block[blockID][blockRotate][i][j] == 1 && i + y >= 0)
			{
				move(i + y + 1, j + x + 1);
				attron(A_REVERSE);
				printw("%c", tile);
				attroff(A_REVERSE);
			}
		}

	move(HEIGHT, WIDTH + 10);
}

void DrawBox(int y, int x, int height, int width)
{
	int i, j;
	move(y, x);
	addch(ACS_ULCORNER);
	for (i = 0; i < width; i++)
		addch(ACS_HLINE);
	addch(ACS_URCORNER);
	for (j = 0; j < height; j++)
	{
		move(y + j + 1, x);
		addch(ACS_VLINE);
		move(y + j + 1, x + width + 1);
		addch(ACS_VLINE);
	}
	move(y + j + 1, x);
	addch(ACS_LLCORNER);
	for (i = 0; i < width; i++)
		addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void play()
{
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM, &act, &oact);
	InitTetris();
	do
	{
		if (timed_out == 0)
		{
			alarm(1);
			timed_out = 1;
		}

		command = GetCommand();
		if (ProcessCommand(command) == QUIT)
		{
			alarm(0);
			DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
			move(HEIGHT / 2, WIDTH / 2 - 4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
	} while (!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
	move(HEIGHT / 2, WIDTH / 2 - 4);
	printw("GameOver!!");
	refresh();
	getch();
	newRank(score);
}

char menu()
{
	Node *temp = head;
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");

	return wgetch(stdscr);
}

/////////////////////////첫주차 실습에서 구현해야 할 함수/////////////////////////

int CheckToMove(char f[HEIGHT][WIDTH], int currentBlock, int blockRotate, int blockY, int blockX)
{
	// user code
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (block[currentBlock][blockRotate][i][j] == 1)
			{
				if (!(((0 <= i + blockY) && (i + blockY < HEIGHT)) && ((0 <= j + blockX) && (j + blockX < WIDTH))))
				{
					return 0;
				}

				if (f[i + blockY][j + blockX] == 1)
				{
					return 0;
				}
			}
		}
	}

	return 1;
}

void DrawChange(char f[HEIGHT][WIDTH], int command, int currentBlock, int blockRotate, int blockY, int blockX)
{
	// user code

	// 1. 이전 블록 정보를 찾는다. ProcessCommand의 switch문을 참조할 것
	// 2. 이전 블록 정보를 지운다. DrawBlock함수 참조할 것.
	// 3. 새로운 블록 정보를 그린다.
	int x = blockX, y = blockY, r = blockRotate;
	int k;
	switch (command)
	{
	case KEY_UP:
		r = (blockRotate + 3) % 4;
		break;
	case KEY_DOWN:
		y = blockY - 1;
		break;
	case KEY_RIGHT:
		x = blockX - 1;
		break;
	case KEY_LEFT:
		x = blockX + 1;
		break;
	default:
		break;
	}

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (block[currentBlock][r][i][j] == 1 && i + y >= 0)
			{
				move(i + y + 1, j + x + 1);
				printw(".");
			}
		}
	}

	for (k = 0; k < HEIGHT; k++)
	{
		if (CheckToMove(field, currentBlock, r, y + k + 1, x) == 0)
		{
			break;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (block[currentBlock][r][i][j] == 1 && i + y >= 0)
			{
				move(i + k + y + 1, j + x + 1);
				printw(".");
			}
		}
	}

	DrawBlockWithFeatures(blockY, blockX, currentBlock, blockRotate);

	move(HEIGHT, WIDTH + 10);
}

void BlockDown(int sig)
{
	// user code

	// 강의자료 p26-27의 플로우차트를 참고한다.
	if (CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX))
	{
		blockY++;
		DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX);
	}

	else
	{
		if (blockY == -1)
		{
			gameOver = 1;
		}
		else
		{
			AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX);
			score += DeleteLine(field) + touched * 10;
			for(int i=0;i<BLOCK_NUM-1;i++)
			{
				nextBlock[i]=nextBlock[i+1];
			}
			nextBlock[BLOCK_NUM-1]= rand()%7;
			blockRotate = 0;
			blockY = -1;
			blockX = WIDTH / 2 - 2;

			recRoot->lv = 0;
			recRoot->score = 0;
			for (int i = 0; i < HEIGHT; i++)
			{
				for (int j = 0; j < WIDTH; j++)
				{
					recRoot->f[i][j] = field[i][j];
				}
			}

			for(int i=0;i<CHILDREN_MAX;i++){
				recRoot->c[i]=NULL;
			}
			accumulatedscore = 0;
			recommendX=0;
			recommendY=0;
			recommendR=0;
			recommend(recRoot);
			DrawNextBlock(nextBlock);
			PrintScore(score);
			DrawField();
			DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
		}
	}
	timed_out = 0;
}

void AddBlockToField(char f[HEIGHT][WIDTH], int currentBlock, int blockRotate, int blockY, int blockX)
{
	// user code

	// Block이 추가된 영역의 필드값을 바꾼다.
	touched = 0;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (block[currentBlock][blockRotate][i][j] == 1)
			{
				f[i + blockY][j + blockX] = 1;

				if (f[i + blockY + 1][j + blockX] == 1 || (i + blockY == HEIGHT - 1))
				{
					touched++;
				}
			}
		}
	}
}

int DeleteLine(char f[HEIGHT][WIDTH])
{
	// user code

	// 1. 필드를 탐색하여, 꽉 찬 구간이 있는지 탐색한다.
	// 2. 꽉 찬 구간이 있으면 해당 구간을 지운다. 즉, 해당 구간으로 필드값을 한칸씩 내린다.
	int deleted = 0;
	int flag;

	for (int i = HEIGHT - 1; i >= 0; i--)
	{
		flag = 1;
		for (int j = 0; j < WIDTH; j++)
		{
			if (f[i][j] != 1)
			{
				flag = 0;
				break;
			}
		}

		if (flag == 1)
		{
			deleted++;
			for (int k = i; k > 0; k--)
			{
				for (int l = 0; l < WIDTH; l++)
				{
					f[k][l] = f[k - 1][l];
				}
			}

			for (int m = 0; m < WIDTH; m++)
			{
				f[0][m] = 0;
			}
			i++;
		}
	}

	return deleted * deleted * 100;
}

///////////////////////////////////////////////////////////////////////////

void DrawShadow(int y, int x, int blockID, int blockRotate)
{
	// user code
	int i;
	for (i = 0; i < HEIGHT; i++)
	{
		if (CheckToMove(field, blockID, blockRotate, y + i + 1, x) == 0)
		{
			break;
		}
	}

	DrawBlock(y + i, x, blockID, blockRotate, '/');
}

void DrawBlockWithFeatures(int y, int x, int blockID, int blockRatate)
{	
	if(rflag==0)
		DrawShadow(y, x, blockID, blockRatate);
	
	DrawBlock(y, x, blockID, blockRatate, ' ');
	DrawRecommend(recommendY, recommendX, nextBlock[0], recommendR);
}

void createRankList()
{
	// 목적: Input파일인 "rank.txt"에서 랭킹 정보를 읽어들임, 읽어들인 정보로 랭킹 목록 생성
	// 1. "rank.txt"열기
	// 2. 파일에서 랭킹정보 읽어오기
	// 3. LinkedList로 저장
	// 4. 파일 닫기
	FILE *fp;
	int i, j;
	Node *temp;
	Node *curr;
	// 1. 파일 열기
	fp = fopen("rank.txt", "r");

	if (fp == NULL)
	{
		printw("file open error\n");
		fp = fopen("rank.txt", "w");
		fprintf(fp, "%d\n", score_number);
	}

	// 2. 정보읽어오기
	/* int fscanf(FILE* stream, const char* format, ...);
	stream:데이터를 읽어올 스트림의 FILE 객체를 가리키는 파일포인터
	format: 형식지정자 등등
	변수의 주소: 포인터
	return: 성공할 경우, fscanf 함수는 읽어들인 데이터의 수를 리턴, 실패하면 EOF리턴 */
	// EOF(End Of File): 실제로 이 값은 -1을 나타냄, EOF가 나타날때까지 입력받아오는 if문
	if (fscanf(fp, "%d", &score_number) != EOF)
	{

		if (head == NULL)
		{
			head = malloc(sizeof(Node));
			fscanf(fp, "%s %d", head->name, &head->score);
			head->link = NULL;
		}

		curr = head;

		for (i = 1; i < score_number; i++)
		{
			temp = malloc(sizeof(Node));
			fscanf(fp, "%s %d", temp->name, &temp->score);
			temp->link = NULL;
			curr->link = temp;
			curr = curr->link;
		}
	}

	else
	{

		printw("no Rank!\n");
	}
	// 4. 파일닫기
	fclose(fp);
}

void rank()
{
	// 목적: rank 메뉴를 출력하고 점수 순으로 X부터~Y까지 출력함
	// 1. 문자열 초기화
	int X = 1, Y = score_number, ch, i, j;
	clear();

	// 2. printw()로 3개의 메뉴출력
	printw("1. list ranks from X to Y\n");
	printw("2. list ranks by a specific name\n");
	printw("3. delete a specific rank\n");

	// 3. wgetch()를 사용하여 변수 ch에 입력받은 메뉴번호 저장
	ch = wgetch(stdscr);

	// 4. 각 메뉴에 따라 입력받을 값을 변수에 저장
	// 4-1. 메뉴1: X, Y를 입력받고 적절한 input인지 확인 후(X<=Y), X와 Y사이의 rank 출력
	if (ch == '1')
	{
		Node *temp = head;
		echo();
		printw("X: ");
		scanw("%d", &X);
		printw("Y: ");
		scanw("%d", &Y);
		printw("\t\tname\t\t |\tsocre\n");
		printw("----------------------------------------------\n");
		if (X > Y)
		{
			printw("search failure: no rank in the list\n");
		}

		else
		{
			for (i = 1; i <= Y; i++)
			{
				if (i >= X)
				{
					printw("%-2d|%-30s|%d\n", i, temp->name, temp->score);
				}
				temp = temp->link;
			}
		}
		noecho();
	}

	// 4-2. 메뉴2: 문자열을 받아 저장된 이름과 비교하고 이름에 해당하는 리스트를 출력
	else if (ch == '2')
	{
		char str[NAMELEN + 1];
		int check = 0;
		Node *temp = head;
		echo();
		printw("input the name: ");
		scanw("%s", str);
		noecho();
		printw("\t\tname\t\t|\tsocre\n");
		printw("---------------------------------------------\n");

		for (int i = 0; i < score_number; i++)
		{
			if (!strcmp(temp->name, str))
			{
				printw("%-2d|%-30s|%d\n", i + 1, temp->name, temp->score);
				check++;
			}

			temp = temp->link;
		}

		if (check == 0)
		{
			printw("search failure: no name in the list\n");
		}
	}

	// 4-3. 메뉴3: rank번호를 입력받아 리스트에서 삭제
	else if (ch == '3')
	{
		int num;
		Node *temp = head, *pre;

		printw("input the rank: ");
		echo();
		scanw("%d", &num);
		noecho();

		if (num > score_number)
		{
			printw("search failure: the rank not in the list\n");
		}

		else if (num == 1)
		{
			head = head->link;
			free(temp);
			score_number--;
			printw("result: the rank deleted");
			writeRankFile();
		}

		else
		{
			for (int i = 1; i < num; i++)
			{
				pre = temp;
				temp = temp->link;
			}

			pre->link = temp->link;
			free(temp);
			score_number--;
			printw("result: the rank deleted");
			writeRankFile();
		}
	}

	getch();
}

void writeRankFile()
{
	// 목적: 추가된 랭킹 정보가 있으면 새로운 정보를 "rank.txt"에 쓰고 없으면 종료
	int sn, i;
	// 1. "rank.txt" 연다
	FILE *fp = fopen("rank.txt", "w");
	Node *temp = head;

	if (fp == NULL)
	{
		printw("error\n");
	}
	// 2. 랭킹 정보들의 수를 "rank.txt"에 기록
	fprintf(fp, "%d\n", score_number);
	// 3. 탐색할 노드가 더 있는지 체크하고 있으면 다음 노드로 이동, 없으면 종료

	for (int i = 0; i < score_number; i++)
	{
		fprintf(fp, "%s %d\n", temp->name, temp->score);
		temp = temp->link;
	}
	fclose(fp);
}

void newRank(int score)
{
	// 목적: GameOver시 호출되어 사용자 이름을 입력받고 score와 함께 리스트의 적절한 위치에 저장
	Node *n, *temp, *pre;
	char str[NAMELEN + 1];
	int i = 1, j;
	clear();
	echo();
	// 1. 사용자 이름을 입력받음
	printw("enter the name: ");
	scanw("%s", str);
	// 2. 새로운 노드를 생성해 이름과 점수를 저장, score_number가
	n = malloc(sizeof(Node));
	strcpy(n->name, str);
	n->score = score;
	if (head == NULL)
	{
		head = malloc(sizeof(Node));
	}
	temp = head;
	pre = head;
	noecho();
	score_number++;

	if (score_number == 1)
	{
		head = n;
	}
	else
	{

		for (i; i < score_number; i++)
		{
			if ((temp->score > score))
			{
				pre = temp;
				temp = temp->link;
			}
			else
			{
				break;
			}
		}

		if (i == 1)
		{
			n->link = head;
			head = n;
		}

		else
		{
			n->link = temp;
			pre->link = n;
		}
	}
	writeRankFile();
}

void DrawRecommend(int y, int x, int blockID, int blockRotate)
{
	// user code
	DrawBlock(y, x, blockID, blockRotate, 'R');
}

int recommend(RecNode *root)
{
	int max = 0; // 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수
	int r, x, y, k=0;
	int tempscore;

	for (r = 0; r < 4; r++)
	{
		for (x = -3; x < WIDTH+3; x++)
		{
			if (CheckToMove(root->f, nextBlock[root->lv], r, 0, x) == 0)
				continue;

			RecNode * child;
			child = malloc(sizeof(RecNode));
			child->lv = root->lv +1;
			child->score = root->score;
			
			for (int a = 0; a < HEIGHT; a++)
			{
				for (int j = 0; j < WIDTH; j++)
				{
					child->f[a][j] = root->f[a][j];
				}
			}
			root->c[k] = child;

			for (y = 0; y < HEIGHT; y++)
			{
				if (CheckToMove(child->f, nextBlock[root->lv], r, y + 1, x) == 0)
				{
					break;
				}
			}

			AddBlockToField(child->f, nextBlock[root->lv], r, y, x);
			child->score += touched * 10 + DeleteLine(child->f);
			if (root->lv < BLOCK_NUM-1)
			{
				accumulatedscore = recommend(child);
			}
			
			else {
				accumulatedscore = child->score;
			}

			if (max <= accumulatedscore)
			{	
				max = accumulatedscore;
				if (root->lv == 0)
				{
					recommendX = x;
					recommendY = y;
					recommendR = r;
				}
			}
			k++;
		}
	}
	return max;
}

int modified_recommend(RecNode *root){
	int max = 0; // 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수
	int r, x, y, k=0;
	maxs max_s[10];//상위 10개의 점수를 저장할 구조체

	for(int i=0;i<10;i++){//초기 구조체의 값 초기화
		max_s[i].score=0;
		max_s[i].index=0;
		max_s[i].y=y;
	}

	for (r = 0; r < 4; r++)
	{
		for (x = -3; x < WIDTH+3; x++)
		{
			if (CheckToMove(root->f, nextBlock[root->lv], r, 0, x) == 0)//해당하는 x값이 field에 올 수 있는지 확인
				continue;

			RecNode * child;
			child = malloc(sizeof(RecNode));//메모리 할당
			child->lv = root->lv +1; //자식의 lv을 부모의 lv보다 1 높게 저장
			child->score = root->score; //부모의 점수 자식에 저장
			
			for (int a = 0; a < HEIGHT; a++)//field 복사
			{
				for (int j = 0; j < WIDTH; j++)
				{
					child->f[a][j] = root->f[a][j];
				}
			}
			root->c[k] = child;//child를 root의 자식 포인터에 저장

			for (y = 0; y < HEIGHT; y++)//최대로 내려갈 수 있는 y값 확인
			{
				if (CheckToMove(child->f, nextBlock[root->lv], r, y + 1, x) == 0)
				{
					break;
				}
			}

			AddBlockToField(child->f, nextBlock[root->lv], r, y, x);//필드에 정보 저장
			child->score += touched * 10 + DeleteLine(child->f)*2 + y*10;
			//child에 점수 저장. 빈 공간과 최대한 많은 라인을 지우기 위해  y 값과 deleteline의 점수에 가중치 부여
			child ->x = x;
			child ->y =y;
			child->r= r;//x,y,r에 해당하는 값 저장

			if(child->score > max_s[9].score){//저장되어 있는 최소값보다 작다면 저장하고 내림차순 정렬
				max_s[9].score=child->score;
				max_s[9].index = k;
				max_s[9].c=child;
				max_s[9].y=y;
				qsort(max_s,10, sizeof(max_s[0]), compare);
			}
			k++;
		}
	}

	if(root->lv<BLOCK_NUM-1){
		for(int i=0;i<10;i++){
			accumulatedscore = modified_recommend(root->c[max_s[i].index]);//10개의 최대점수에 대해 재귀함수 호출
			max_s[i].score = accumulatedscore;
			if(max<=accumulatedscore){//max의 값이 accumulatedscore 보다 작거나 같으면 갱신
				max = accumulatedscore;
				qsort(max_s,10, sizeof(max_s[0]), compare);//받아들인 점수에 대햐어 다시 sort
				if(root->lv == 0){
				qsort(max_s,10, sizeof(max_s[0]), compare);//다시 sort
				recommendX = max_s[0].c->x;
				recommendY = max_s[0].c->y;
				recommendR = max_s[0].c->r;//최대값에 대한 x,y,r 값 저장
				}
			}
		}
	}

	else{
		max = max_s[0].score;//root의 lv가 BLOCK_NUM-1이면 max에 최대값 저장
	}
	
	for(int i=0;i<k;i++){
		free(root->c[i]);//동적할당 해제
	}
	return max;
}

void R_BlockDown(int sig)
{
	// user code

	// 강의자료 p26-27의 플로우차트를 참고한다.
	if (CheckToMove(field, nextBlock[0], blockRotate, blockY+1, blockX)==0)
	{
		gameOver=1;
	}

	else
	{
		//if (blockY == -1)
		//{
		//	gameOver = 1;
		//}
		//else
		//{	
			AddBlockToField(field, nextBlock[0], recommendR, recommendY, recommendX);
			score += DeleteLine(field) + touched * 10;
			for(int i=0;i<BLOCK_NUM-1;i++)
			{
				nextBlock[i]=nextBlock[i+1];
			}
			nextBlock[BLOCK_NUM-1]= rand()%7;
			blockRotate = 0;
			blockY = -1;
			blockX = WIDTH / 2 - 2;

			recRoot->lv = 0;
			recRoot->score = 0;
			for (int i = 0; i < HEIGHT; i++)
			{
				for (int j = 0; j < WIDTH; j++)
				{
					recRoot->f[i][j] = field[i][j];
				}
			}

			for(int i=0;i<CHILDREN_MAX;i++){
				recRoot->c[i]=NULL;
			}
			accumulatedscore = 0;
			recommendX=0;
			recommendY=0;
			recommendR=0;
			modified_recommend(recRoot);
			DrawNextBlock(nextBlock);
			PrintScore(score);
			DrawField();
			//DrawBlockWithFeatures(recommendY, recommendX, nextBlock[0], recommendR);
			DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
		//}
	}
	timed_out = 0;
}

void recommendedPlay()
{
	// user code
	int command;
	clear();
	double t;
	start=time(NULL);
	act.sa_handler = R_BlockDown;
	sigaction(SIGALRM, &act, &oact);
	InitTetris();
	do
	{
		if (timed_out == 0)
		{
			alarm(1);
			timed_out = 1;
		}

		command = GetCommand();
		if (ProcessCommand(command) == QUIT)
		{
			alarm(0);
			DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
			move(HEIGHT / 2, WIDTH / 2 - 4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}

		end = time(NULL);
		t = (double)difftime(end,start);

		move(16, WIDTH + 25);
        printw("t\t\t: %10.2f   sec", t);
        move(17, WIDTH + 25);
        printw("score/time\t: %10.2f", score / t);	
	} while (!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
	move(HEIGHT / 2, WIDTH / 2 - 4);
	printw("GameOver!!");
	refresh();
	getch();
	//newRank(score);
}

/*
 *        Computer Graphics Course - Shenzhen University
 *    Mid-term Assignment - Tetris implementation sample code
 * ============================================================
 *
 * - 本代码仅仅是参考代码，具体要求请参考作业说明，按照顺序逐步完成。
 * - 关于配置OpenGL开发环境、编译运行，请参考第一周实验课程相关文档。
 *
 * - 已实现的基础功能如下：
 * - 1) 绘制棋盘格和‘L’型方块
 * - 2) 键盘左/右/下键控制方块的移动，上键旋转方块
 * - 3) 绘制‘J’、‘Z’等形状的方块 @2024-10-23
 * - 4) 随机生成方块并赋上不同的颜色 @2024-10-23
 * - 5) 方块的自动向下移动 @2024-10-23
 * - 6) 方块之间、方块与边界之间的碰撞检测 @2024-10-24
 * - 7) 棋盘格中每一行填充满之后自动消除 @2024-10-24
 * - 8) 占满后游戏结束（能够检测到游戏结束条件）@2024-10-24
 * - 9) 按r键可以重新开始游戏 @2024-10-24
 * 
 * - 已实现的进阶功能如下：
 * 
 * 
 * - 未实现功能如下：
 * - 1) 按空格键可以快速下落
 * - 2) 随着游戏进行，下落速度加快
 * - 3) 按p键可以暂停游戏
 * - 4) 退出游戏和暂停游戏的信息显示
 * - 5) 下落位置的预览
 * - 6) 消除效果的特效
 *
 */

#include "include/Angel.h"

#include <cstdlib>
#include <iostream>
#include <string>

int starttime;			// 控制方块向下移动时间
double startTime; // 记录程序开始的时间（用于计算时间变化）
int rotation = 0;		// 控制当前窗口中的方块旋转
glm::vec2 tile[4];			// 表示当前窗口中的方块
bool gameover = false;	// 游戏结束控制变量
int xsize = 400;		// 窗口大小（尽量不要变动窗口大小！）
int ysize = 720;

// 单个网格大小
int tile_width = 33;

// 网格布大小
const int board_width = 10;
const int board_height = 20;

// 网格三角面片的顶点数量
const int points_num = board_height * board_width * 6;

// 我们用画直线的方法绘制网格
// 包含竖线 board_width+1 条
// 包含横线 board_height+1 条
// 一条线2个顶点坐标
// 网格线的数量
const int board_line_num =  (board_width + 1) + (board_height + 1);

/*
	@TODO: 绘制‘J’、‘Z’等形状的方块
	二维数组表示tile旋转后可能出现的所有位置
	[4][4]，4种旋转，一个tile由四个坐标（小块）形成
*/
// 定义一个包含所有形状旋转数据的二维数组
glm::vec2 allRotations[7][4][4] = {
    // "O"型 tile
    {
        {glm::vec2(0, 0), glm::vec2(0, -1), glm::vec2(-1, 0), glm::vec2(-1,-1)},
        {glm::vec2(0, 0), glm::vec2(0, -1), glm::vec2(-1, 0), glm::vec2(-1,-1)},
        {glm::vec2(0, 0), glm::vec2(0, -1), glm::vec2(-1, 0), glm::vec2(-1,-1)},
        {glm::vec2(0, 0), glm::vec2(0, -1), glm::vec2(-1, 0), glm::vec2(-1,-1)}
    },
    // "I"型 tile
    {
        {glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(-1, 0), glm::vec2(-2, 0)},
        {glm::vec2(0, 0), glm::vec2(0, -1), glm::vec2(0, 1), glm::vec2(0, -2)},
        {glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(-1, 0), glm::vec2(-2, 0)},
        {glm::vec2(0, 0), glm::vec2(0, -1), glm::vec2(0, 1), glm::vec2(0, -2)}
    },
    // "S"型 tile
    {
        {glm::vec2(0, 0), glm::vec2(0, -1), glm::vec2(-1, -1), glm::vec2(1, 0)},
        {glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, -1), glm::vec2(0, 1)},
        {glm::vec2(0, 0), glm::vec2(0, -1), glm::vec2(-1, -1), glm::vec2(1, 0)},
        {glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, -1), glm::vec2(0, 1)}
    },
    // "Z"型 tile
    {
        {glm::vec2(0, 0), glm::vec2(0, -1), glm::vec2(1, -1), glm::vec2(-1, 0)},
        {glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, -1)},
        {glm::vec2(0, 0), glm::vec2(0, -1), glm::vec2(1, -1), glm::vec2(-1, 0)},
        {glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, -1)}
    },
    // "L"型 tile
    {
        {glm::vec2(0, 0), glm::vec2(-1,0), glm::vec2(1, 0), glm::vec2(-1,-1)},
        {glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(0,-1), glm::vec2(1, -1)},
        {glm::vec2(1, 1), glm::vec2(-1,0), glm::vec2(0, 0), glm::vec2(1,  0)},
        {glm::vec2(-1,1), glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(0, -1)}
    },
    // "J"型 tile
    {
        {glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(-1, 0), glm::vec2(1, -1)},
        {glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(0, -1), glm::vec2(1, 1)},
        {glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(-1, 0), glm::vec2(-1, 1)},
        {glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(0, -1), glm::vec2(-1, -1)}
    },
    // "T"型 tile
    {
        {glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(-1, 0), glm::vec2(0, -1)},
        {glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(0, -1), glm::vec2(1, 0)},
        {glm::vec2(0, 0), glm::vec2(-1, 0), glm::vec2(1, 0), glm::vec2(0, 1)},
        {glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(0, -1), glm::vec2(-1, 0)}
    }
};

// 绘制窗口的颜色变量
glm::vec4 orange = glm::vec4(1.0, 0.5, 0.0, 1.0);
glm::vec4 white  = glm::vec4(1.0, 1.0, 1.0, 1.0);
glm::vec4 black  = glm::vec4(0.0, 0.0, 0.0, 1.0);

// 可供方块选择的颜色
glm::vec4 colors[] = {
	glm::vec4(0.976, 0.290, 0.247, 1.0), // Red rgb(249, 74, 63)
	glm::vec4(0.996, 0.980, 0.243, 1.0), // Yellow rgb(254, 250, 62)
	glm::vec4(1.0, 0.627, 0.235, 1.0), // Orange rgb(255, 160, 60)
	glm::vec4(1.0, 0.541, 1.0, 1.0), // Purple rgb(255, 138, 255)
	glm::vec4(0.502, 0.525, 0.996, 1.0) // Blue rgb(128, 134, 254)
};

// 当前方块的位置（以棋盘格的左下角为原点的坐标系）
glm::vec2 tilepos = glm::vec2(5, 19);

// 当前方块的形状
int tileshape; // 0~6

// 当前方块的颜色
glm::vec4 tilecolor;


// 布尔数组表示棋盘格的某位置是否被方块填充，即board[x][y] = true表示(x,y)处格子被填充。
// （以棋盘格的左下角为原点的坐标系）
bool board[board_width][board_height];

// 当棋盘格某些位置被方块填充之后，记录这些位置上被填充的颜色
glm::vec4 board_colours[points_num];

GLuint locxsize;
GLuint locysize;

GLuint vao[3];
GLuint vbo[6];

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// 修改棋盘格在pos位置的颜色为colour，并且更新对应的VBO
void changecellcolour(glm::vec2 pos, glm::vec4 colour)
{
	// 每个格子是个正方形，包含两个三角形，总共6个定点，并在特定的位置赋上适当的颜色
	for (int i = 0; i < 6; i++)
		board_colours[(int)( 6 * ( board_width*pos.y + pos.x) + i)] = colour;

	glm::vec4 newcolours[6] = {colour, colour, colour, colour, colour, colour};

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);

	// 计算偏移量，在适当的位置赋上颜色
	int offset = 6 * sizeof(glm::vec4) * (int)( board_width * pos.y + pos.x);
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// 当前方块移动或者旋转时，更新VBO
void updatetile()
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);

	// 每个方块包含四个格子
	for (int i = 0; i < 4; i++)
	{
		// 计算格子的坐标值
		GLfloat x = tilepos.x + tile[i].x;
		GLfloat y = tilepos.y + tile[i].y;

		glm::vec4 p1 = glm::vec4(tile_width + (x * tile_width), tile_width + (y * tile_width), .4, 1);
		glm::vec4 p2 = glm::vec4(tile_width + (x * tile_width), tile_width*2 + (y * tile_width), .4, 1);
		glm::vec4 p3 = glm::vec4(tile_width*2 + (x * tile_width), tile_width + (y * tile_width), .4, 1);
		glm::vec4 p4 = glm::vec4(tile_width*2 + (x * tile_width), tile_width*2 + (y * tile_width), .4, 1);

		// 每个格子包含两个三角形，所以有6个顶点坐标
		glm::vec4 newpoints[6] = {p1, p2, p3, p2, p3, p4};
		glBufferSubData(GL_ARRAY_BUFFER, i*6*sizeof(glm::vec4), 6*sizeof(glm::vec4), newpoints);
	}
	glBindVertexArray(0);
	
}

// 设置当前方块为下一个即将出现的方块。在游戏开始的时候调用来创建一个初始的方块，
// 在游戏结束的时候判断，没有足够的空间来生成新的方块。
/*
	@TODO: 随机生成方块并赋上不同的颜色
*/
void newtile()
{
	// 将新方块放于棋盘格的最上行中间位置并设置默认的旋转方向
	tilepos = glm::vec2(5 , 19);
	rotation = 0;

	/* 随机生成不同形状的方块，使用随机数 */
	tileshape = rand() % 7;
	for (int i = 0; i < 4; ++i) {
		tile[i] = allRotations[tileshape][0][i];
	}

	/*
		在生成形状和位置后，检测每个坐标是否被占满。若占满，则说明没有足够空间生成新的方块
	*/
	for (int i = 0; i < 4; ++i) {
		int x = tilepos.x + tile[i].x; // 参考updatetitle知道如何计算方块的小格的坐标值
		int y = tilepos.y + tile[i].y;
		if (board[x][y]) {
			exit(EXIT_SUCCESS);  // 结束游戏
		}
	}


	updatetile();

	/* 随机附上颜色 */
	tilecolor = colors[rand() % 5];

	// 给新方块赋上颜色
	glm::vec4 newcolours[24];
	// 一小块有6个点 一个tile就有24个点需要附上颜色
	for (int i = 0; i < 24; i++)
		newcolours[i] = tilecolor;

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

// 游戏和OpenGL初始化
void init()
{
	// 初始化棋盘格，这里用画直线的方法绘制网格
	// 包含竖线 board_width+1 条
	// 包含横线 board_height+1 条
	// 一条线2个顶点坐标，并且每个顶点一个颜色值
	
	glm::vec4 gridpoints[board_line_num * 2];
	glm::vec4 gridcolours[board_line_num * 2];

	// 绘制网格线
	// 纵向线
	for (int i = 0; i < (board_width+1); i++)
	{
		gridpoints[2*i] = glm::vec4((tile_width + (tile_width * i)), tile_width, 0, 1);
		gridpoints[2*i + 1] = glm::vec4((tile_width + (tile_width * i)), (board_height+1) * tile_width, 0, 1);
	}

	// 水平线
	for (int i = 0; i < (board_height+1); i++)
	{
		gridpoints[ 2*(board_width+1) + 2*i ] = glm::vec4(tile_width, (tile_width + (tile_width * i)), 0, 1);
		gridpoints[ 2*(board_width+1) + 2*i + 1 ] = glm::vec4((board_width+1) * tile_width, (tile_width + (tile_width * i)), 0, 1);
	}

	// 将所有线赋成白色
	for (int i = 0; i < (board_line_num * 2); i++)
		gridcolours[i] = white;

	// 初始化棋盘格，并将没有被填充的格子设置成黑色
	glm::vec4 boardpoints[points_num];
	for (int i = 0; i < points_num; i++)
		board_colours[i] = black;

	// 对每个格子，初始化6个顶点，表示两个三角形，绘制一个正方形格子
	for (int i = 0; i < board_height; i++)
		for (int j = 0; j < board_width; j++)
		{
			glm::vec4 p1 = glm::vec4(tile_width + (j * tile_width), tile_width + (i * tile_width), .5, 1);
			glm::vec4 p2 = glm::vec4(tile_width + (j * tile_width), tile_width*2 + (i * tile_width), .5, 1);
			glm::vec4 p3 = glm::vec4(tile_width*2 + (j * tile_width), tile_width + (i * tile_width), .5, 1);
			glm::vec4 p4 = glm::vec4(tile_width*2 + (j * tile_width), tile_width*2 + (i * tile_width), .5, 1);
			boardpoints[ 6 * ( board_width * i + j ) + 0 ] = p1;
			boardpoints[ 6 * ( board_width * i + j ) + 1 ] = p2;
			boardpoints[ 6 * ( board_width * i + j ) + 2 ] = p3;
			boardpoints[ 6 * ( board_width * i + j ) + 3 ] = p2;
			boardpoints[ 6 * ( board_width * i + j ) + 4 ] = p3;
			boardpoints[ 6 * ( board_width * i + j ) + 5 ] = p4;
		}

	// 将棋盘格所有位置的填充与否都设置为false（没有被填充）
	for (int i = 0; i < board_width; i++)
		for (int j = 0; j < board_height; j++)
			board[i][j] = false;

	// 载入着色器
	std::string vshader, fshader;
	vshader = "shaders/vshader.glsl";
	fshader = "shaders/fshader.glsl";
	GLuint program = InitShader(vshader.c_str(), fshader.c_str());
	glUseProgram(program);

	locxsize = glGetUniformLocation(program, "xsize");
	locysize = glGetUniformLocation(program, "ysize");

	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	GLuint vColor = glGetAttribLocation(program, "vColor");

	
	glGenVertexArrays(3, &vao[0]);
	glBindVertexArray(vao[0]);		// 棋盘格顶点
	
	glGenBuffers(2, vbo);

	// 棋盘格顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, (board_line_num * 2) * sizeof(glm::vec4), gridpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// 棋盘格顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, (board_line_num * 2) * sizeof(glm::vec4), gridcolours, GL_STATIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	
	glBindVertexArray(vao[1]);		// 棋盘格每个格子

	glGenBuffers(2, &vbo[2]);

	// 棋盘格每个格子顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, points_num*sizeof(glm::vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// 棋盘格每个格子顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, points_num*sizeof(glm::vec4), board_colours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	
	glBindVertexArray(vao[2]);		// 当前方块

	glGenBuffers(2, &vbo[4]);

	// 当前方块顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// 当前方块顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	
	glBindVertexArray(0);

	glClearColor(0, 0, 0, 0);

	// 游戏初始化
	newtile();
	// starttime = glutGet(GLUT_ELAPSED_TIME);
}

/*
	实现方块与方块之间、方块与边界之间的碰撞检测
*/
// 检查在cellpos位置的格子是否被填充或者是否在棋盘格的边界范围内
bool checkvalid(glm::vec2 cellpos)
{
	if((cellpos.x >=0) && (cellpos.x < board_width) && (cellpos.y >= 0) && (cellpos.y < board_height) &&
		!board[static_cast<int>(cellpos.x)][static_cast<int>(cellpos.y)])
		return true;
	else
		return false;
}

// 在棋盘上有足够空间的情况下旋转当前方块
void rotate()
{
	// 计算得到下一个旋转方向
	int nextrotation = (rotation + 1) % 4;

	// 检查当前旋转之后的位置的有效性
	if (checkvalid((allRotations[tileshape][nextrotation][0]) + tilepos)
		&& checkvalid((allRotations[tileshape][nextrotation][1]) + tilepos)
		&& checkvalid((allRotations[tileshape][nextrotation][2]) + tilepos)
		&& checkvalid((allRotations[tileshape][nextrotation][3]) + tilepos))
	{
		// 更新旋转，将当前方块设置为旋转之后的方块
		rotation = nextrotation;
		for (int i = 0; i < 4; i++)
			tile[i] = allRotations[tileshape][rotation][i];

		updatetile();
	}
}

/*
	检查棋盘格在row行有没有被填充满（若填满，则消除该行，并将上方所有方块向下移动一行）
	并返回full，如果true则row行满且完成了消除，若false则row行非满
*/
bool checkfullrow(int row)
{
	// 检查row行是否被填满
	bool full = true;
	for (int col = 0; col < board_width; ++col) {
		if (!board[col][row]) {
			full = false;
			break;
		}
	}

	// 若row行被填满
	if (full) {

		// 消除该行
		for (int col = 0; col < board_width; ++col) {
			board[col][row] = false; // 恢复为没有填充的状态
			changecellcolour(glm::vec2(col, row), black); // 改变颜色为默认初始的黑色
		}

		// 该行上方所有方块向下移动一行
		for (int curr_row = row; curr_row < board_height - 1; ++curr_row) {
			for (int curr_col = 0; curr_col < board_width; ++curr_col) {
				board[curr_col][curr_row] = board[curr_col][curr_row + 1];
				// 获取 (curr_col, curr_row + 1)的颜色并赋值
				changecellcolour(glm::vec2(curr_col, curr_row), board_colours[(int)( 6 * ( board_width * (curr_row + 1) + curr_col))]); 
			}
		}

		// 消除第board_height - 1行（top行）
		for (int col = 0; col < board_width; ++col) {
			board[col][board_height - 1] = false; // 恢复为没有填充的状态
			changecellcolour(glm::vec2(col, board_height - 1), black); // 改变颜色为默认初始的黑色
		}
	}

	return full;
}

// 放置当前方块（固定在特定的位置），并且更新棋盘格对应位置顶点的颜色VBO
void settile()
{
	// 每个格子
	for (int i = 0; i < 4; i++)
	{
		// 获取格子在棋盘格上的坐标
		int x = (tile[i] + tilepos).x;
		int y = (tile[i] + tilepos).y;
		// 将格子对应在棋盘格上的位置设置为填充
		board[x][y] = true;
		// 并将相应位置的颜色修改
		changecellcolour(glm::vec2(x, y), tilecolor);
	}

	// 检测每一行是否被填满
	for (int r = 0; r < board_height; ++r) {
		if (checkfullrow(r)) {
			r--; // 若i行满且完成了消除，则应对i行再进行一次检查
		}
	}
}

// 给定位置(x,y)，移动方块。有效的移动值为(-1,0)，(1,0)，(0,-1)，分别对应于向
// 左，向右和向下移动。如果移动成功，返回值为true，反之为false
// Ps: 此处原注释是错误的，向右和向下写反了，已修正
bool movetile(glm::vec2 direction)
{
	// 计算移动之后的方块的位置坐标
	glm::vec2 newtilepos[4];
	for (int i = 0; i < 4; i++)
		newtilepos[i] = tile[i] + tilepos + direction;

	// 检查移动之后的有效性
	if (checkvalid(newtilepos[0])
		&& checkvalid(newtilepos[1])
		&& checkvalid(newtilepos[2])
		&& checkvalid(newtilepos[3]))
		{
			// 有效：移动该方块
			tilepos.x = tilepos.x + direction.x;
			tilepos.y = tilepos.y + direction.y;

			updatetile();

			return true;
		}

	return false;
}

// 重新启动游戏
void restart()
{
	init(); // 重新初始化
	gameover = false; // 重新设置gameover为false（使得游戏结束后可以重新开始游戏）
	startTime = glfwGetTime(); // 更新时间
}

// 游戏渲染部分
void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize);
	glUniform1i(locysize, ysize);

	glBindVertexArray(vao[1]);
	glDrawArrays(GL_TRIANGLES, 0, points_num); // 绘制棋盘格 (width * height * 2 个三角形)
	glBindVertexArray(vao[2]);
	glDrawArrays(GL_TRIANGLES, 0, 24);	 // 绘制当前方块 (8 个三角形)
	glBindVertexArray(vao[0]);
	glDrawArrays(GL_LINES, 0, board_line_num * 2 );		 // 绘制棋盘格的线

}

// 在窗口被拉伸的时候，控制棋盘格的大小，使之保持固定的比例。
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

/*
	实现方块的快速下落功能
*/
void droptile() {
	while (movetile(glm::vec2(0, -1))) {
		// 只要有效移动，就不断向下
	}
	settile(); // 降到最低位置后，安置方块
	newtile(); // 刷新新的方块
}

// 键盘响应事件中的特殊按键响应
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if(!gameover)
	{
		switch(key)
		{	
			// 控制方块的移动方向，更改形态
			case GLFW_KEY_UP:	// 向上按键旋转方块
				if (action == GLFW_PRESS || action == GLFW_REPEAT)
				{
					rotate();
					break;
				}
				else
				{
					break;
				}
			case GLFW_KEY_DOWN: // 向下按键移动方块
				if (action == GLFW_PRESS || action == GLFW_REPEAT){
					// 移动失败
					if (!movetile(glm::vec2(0, -1)))
					{
						settile(); // 放置
						newtile(); // 新tile
						break;
					}
					// 移动成功
					else
					{
						break;
					}
				}
			case GLFW_KEY_LEFT:  // 向左按键移动方块
				if (action == GLFW_PRESS || action == GLFW_REPEAT){
					movetile(glm::vec2(-1, 0));
					break;
				}
				else
				{
					break;
				}
			case GLFW_KEY_RIGHT: // 向右按键移动方块
				if (action == GLFW_PRESS || action == GLFW_REPEAT){
					movetile(glm::vec2(1, 0));
					break;
				}
				else
				{
					break;
				}
			// 游戏设置。
			case GLFW_KEY_ESCAPE:
				if(action == GLFW_PRESS){
					exit(EXIT_SUCCESS);
					break;
				}
				else
				{
					break;
				}
			case GLFW_KEY_Q:
				if(action == GLFW_PRESS){
					exit(EXIT_SUCCESS); // 结束游戏
					break;
				}
				else
				{
					break;
				}
				
			case GLFW_KEY_R:
				if(action == GLFW_PRESS){
					restart(); // 重新启动游戏
					break;
				}
				else
				{
					break;
				}		
			case GLFW_KEY_SPACE:
				if (action == GLFW_PRESS || action == GLFW_REPEAT) {
					droptile();
					break;
				}
				else {
					break;
				}
		}
	}
}



int main(int argc, char **argv)
{
	srand(time(NULL)); // 初始化随机数生成器

	startTime = glfwGetTime(); // 将startTime赋值为程序的初始时间

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
	#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	// 创建窗口。
	GLFWwindow* window = glfwCreateWindow(500, 900, "Mid-Term-Skeleton-Code", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window!" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
	
	
	init();
	while (!glfwWindowShouldClose(window))
    { 
		double currentTime = glfwGetTime();
		if (currentTime - startTime >= 1.0) {
			// 若已经过去了一秒就向下移动方块
			/*
				增加对移动失败的检测，解决到达底部后，不再自动下落引起的判定bug
			*/
			if (!movetile(glm::vec2(0, -1))) {
				settile();
				newtile();
			}
			startTime = currentTime;
		}

        display();
        glfwSwapBuffers(window);
        glfwPollEvents();	
    }
    glfwTerminate();
    return 0;
}

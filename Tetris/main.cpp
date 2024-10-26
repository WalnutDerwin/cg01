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
 * - 1) 按空格键可以快速下落 @2024-10-24
 * - 2) 按p键可以暂停游戏 @2024-10-25
 * - 3) 显示下一个方块 @2024-10-26
 * 
 * - 未实现功能如下：
 * - 1) 随着游戏进行，下落速度加快
 * - 2) 退出游戏和暂停游戏的信息显示
 * - 3) 下落位置的预览
 * - 4) 消除效果的特效
 * - 5) 
 */

#include "include/Angel.h"

#include <cstdlib>
#include <iostream>
#include <string>

/**
 * @brief Tetris基础设定相关的全局变量
 * 
 * 这些变量用于控制游戏的基本设置，包括窗口大小、网格大小、顶点数量和网格填充与否等。
 * 
 * @details
 * - 'starttime': 控制方块向下移动时间（未(在init函数中)启用）
 * - 'startTime': 记录操作开始的时间
 * - 'xsize': 游戏界面的宽度
 * - 'ysize': 游戏界面的高度
 * - 'tile_width': 单个网格的大小
 * - 'board_width': 网格布的宽
 * - 'board_height': 网格布的高
 * - 'board_line_num': 网格线的数量
 * - 'points_num': 网格三角面片的顶点数量
 * - 'board[][]': 表示棋盘格的某位置是否被方块填充 
 * - 'board_colours[]': 记录棋盘格被填充的颜色
 */
int starttime; 														// 控制方块向下移动时间
double startTime; 													// 记录程序开始的时间（用于计算每步操作的时间变化，实现自动下落功能）
int xsize = 400 * 1.6; 												// 游戏界面宽度，扩大1.6倍以显示更多信息
int ysize = 720; 													// 游戏界面高度
int tile_width = 33;												// 单个网格大小
const int board_width = 10; 										// 网格布的宽
const int board_height = 20; 										// 网格布的高
const int board_line_num =  (board_width + 1) + (board_height + 1); // 网格线的数量，竖线 board_width+1 条，横线 board_height+1 条
const int points_num = board_height * board_width * 6; 				// 网格三角面片的顶点数量（一个网格六个顶点）
bool board[board_width][board_height]; 								// 表示棋盘格的某位置是否被方块填充，true则为被填充，采用以棋盘格的左下角为原点的坐标系
glm::vec4 board_colours[points_num]; 								// 当棋盘格某些位置被方块填充之后，记录这些位置上被填充的颜色

/**
 * @brief 绘制相关的全局变量
 * 
 * - 'locxsize': represents the location of the x-size uniform
 * - 'locysize': represents the location of the y-size uniform
 */
GLuint locxsize; 
GLuint locysize;
GLuint vao[3]; 		// 用于绘制当前方块
GLuint vbo[6];
GLuint next_vao; 	// 用于绘制下一个方块
GLuint next_vbo[2];
glm::vec4 orange = glm::vec4(1.0, 0.5, 0.0, 1.0); // 绘制窗口的颜色变量
glm::vec4 white  = glm::vec4(1.0, 1.0, 1.0, 1.0);
glm::vec4 black  = glm::vec4(0.0, 0.0, 0.0, 1.0);

/**
 * @brief 表示方块的全局变量
 * 
 * 这些变量用于表示一个方块的位置、旋转、形状和颜色等。
 * 
 * @details
 * - 'tile[4]': 表示当前方块
 * - 'next_tile[4]': 表示下一个方块
 * - 'rotation': 表示当前方块的旋转
 * - 'tilepos': 表示当前方块的位置
 * - 'next_tilepos': 表示下一方块的位置
 * - 'tileshape': 表示当前方块的形状
 * - 'next_tileshape': 表示下一方块的形状
 * - 'tilecolor': 表示当前方块的颜色
 * - 'next_tilecolor': 表示下一方块的颜色
 * - 'isFirstTile': 表示是否为第一个方块
 */
glm::vec2 tile[4];           // 表示当前方块，每个元素是一个坐标（表示的是初始方块，每个元素需加上tilepos才表示真正的位置）
glm::vec2 next_tile[4];      // 表示下一个方块
int rotation = 0;            // 控制当前窗口中的方块旋转
glm::vec2 tilepos = glm::vec2(5, 19);       // 表示当前方块的位置（以棋盘格的左下角为原点的坐标系）
glm::vec2 next_tilepos = glm::vec2(13, 10); // 表示下一方块的位置
int tileshape;               // 表示当前方块的形状，对应allRotations数组的第一个索引
int next_tileshape;          // 表示下一方块的形状
glm::vec4 tilecolor;         // 表示当前方块的颜色
glm::vec4 next_tilecolor;    // 表示下一个方块的颜色
bool isFirstTile = true;     // 记录是否为第一个方块

/**
 * @var glm::vec2 allRotations[i][j][k]
 * @brief 表示tile旋转后的所有位置
 * 
 * 该数组表示了'O', 'I', 'S', 'Z', 'L', 'J'和'T'形状的方块，并描述了它们至多四种旋转状态。
 * 
 * @details
 * i表示形状（0~6代表O, I等形状）
 * j表示旋转状态（代表某个形状的旋转状态）
 * k表示点的位置（代表某个形状旋转状态下的坐标数组）
 */
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

/**
 * @brief 表示游戏状态的全局变量
 * 
 * 描述游戏的状态，包括结束、暂停和快速下落。
 * 
 * @details
 * - 'gameover': 记录游戏是否结束，true表示游戏结束
 * - 'isPaused': 记录游戏是否暂停，true表示游戏暂停
 * 
 * - 'isDropping': 表示方块处于快速下落状态
 * - 'quickDropInterval': 控制快速下落的时间间隔
 */
bool gameover = false;    // 表示游戏结束与否
bool isPaused = false;    // 表示游戏是否暂停
bool isDropping = false;  // 记录是否在快速下落
double quickDropInterval = 1.0 / 90.0; // 设置掉落的动画帧数为90帧

/**
 * @var glm::vec4 colors[]
 * @brief 表示所有方块可能填充的颜色
 * 
 * @details
 * 所有颜色都提取了word文档上的颜色
 * 类似于shape的选择，颜色的随机选择也是依靠于colors数组的索引
 */
glm::vec4 colors[] = {
	glm::vec4(0.976, 0.290, 0.247, 1.0), // Red rgb(249, 74, 63)
	glm::vec4(0.996, 0.980, 0.243, 1.0), // Yellow rgb(254, 250, 62)
	glm::vec4(1.0, 0.627, 0.235, 1.0), // Orange rgb(255, 160, 60)
	glm::vec4(1.0, 0.541, 1.0, 1.0), // Purple rgb(255, 138, 255)
	glm::vec4(0.502, 0.525, 0.996, 1.0) // Blue rgb(128, 134, 254)
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

/**
 * @brief 修改颜色并更新VBO的函数
 * 
 * @param pos 棋盘格上的位置
 * @param colour 目标颜色
 * 
 * @details
 * 修改棋盘格在pos位置的颜色为colour，并且更新对应的VBO
 */
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

// 更新下一个方块的VBO
void updateNextTile() {
	glBindBuffer(GL_ARRAY_BUFFER, next_vbo[0]);

	// 每个方块包含四个格子
	for (int i = 0; i < 4; i++)
	{
		// 计算格子的坐标值
		GLfloat x = next_tilepos.x + next_tile[i].x;
		GLfloat y = next_tilepos.y + next_tile[i].y;

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

/**
 * @brief 生成新方块的函数
 * 
 * @details
 * 判断是否为第一个方块：若是，则随机生成；否则，将下一个方块赋值给当前方块。
 * 判断有没有足够空间来生成新的方块。（判断游戏结束的条件）
 */
void newtile()
{
	// 若为第一个方块
	if (isFirstTile) {
		tileshape = rand() % 7; // 随机生成不同形状的方块，使用随机数
		tilecolor = colors[rand() % 5]; // 随机附上颜色
		isFirstTile = false;
	}
	else {
		// 非第一方块时，设置当前方块为下一方块
		tileshape = next_tileshape;
		tilecolor = next_tilecolor;
	}

	// 更新下一方块
	next_tileshape = rand() % 7;
	next_tilecolor = colors[rand() % 5];

	// 将新方块放于棋盘格的最上行中间位置并设置默认的旋转方向
	tilepos = glm::vec2(5 , 19);
	rotation = 0;

	// 方块的形状
	for (int i = 0; i < 4; ++i) {
		tile[i] = allRotations[tileshape][0][i];
	}

	// 下一方块的形状
	for (int i = 0; i < 4; ++i) {
		next_tile[i] = allRotations[next_tileshape][0][i];
	}

	// 在生成形状和位置后，检测每个坐标是否被占满。若占满，则说明没有足够空间生成新的方块
	for (int i = 0; i < 4; ++i) {
		int x = tilepos.x + tile[i].x; // 参考updatetitle知道如何计算方块的小格的坐标值
		int y = tilepos.y + tile[i].y;
		if (board[x][y]) {
			exit(EXIT_SUCCESS);  // 结束游戏
		}
	}

	updatetile();
	updateNextTile(); // 更新下一个方块

	// 给新方块赋上颜色
	glm::vec4 newcolours[24];
	// 一小块有6个点 一个tile就有24个点需要附上颜色
	for (int i = 0; i < 24; i++)
		newcolours[i] = tilecolor;

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// 给下一个方块赋上颜色
	for (int i = 0; i < 24; ++i) {
		newcolours[i] = next_tilecolor;
	}

	glBindBuffer(GL_ARRAY_BUFFER, next_vbo[1]);
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

	// 下一个方块
	glGenVertexArrays(1, &next_vao); // new
	glBindVertexArray(next_vao);
	glGenBuffers(2, &next_vbo[0]);

	// 下一个方块顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, next_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// 下一个方块的顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, next_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	glBindVertexArray(0);

	glClearColor(0, 0, 0, 0);

	// 游戏初始化
	newtile();
	// starttime = glutGet(GLUT_ELAPSED_TIME);
}

/**
 * @brief 判断方块的位置是否合法
 * 
 * @param cellpos 判断的方块位置
 * @return bool 合法则返回true，否则false
 * 
 * @details
 * 检测方块与方块，方块与边界之间的碰撞，也就是检查在cellpos位置的格子是否被填充或者是否在棋盘格的边界范围内
 */
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

/**
 * @brief 检查是否行满并消除满行的函数
 * 
 * @param row 检查的行
 * @return bool 若返回true，则说明row行满且完成消除；若为false，则row行非满
 * 
 * @details
 * 消除操作：清空该行，并将上方所有方块向下移动一行
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

/**
 * @brief 放置当前方块在特定位置，并更新棋盘格对应位置顶点的颜色VBO
 */
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

/**
 * @brief 移动方块的函数
 * 
 * @param direction 移动的方向值
 * @return bool 移动成功则返回true
 * 
 * @details
 * 有效的移动值有向左(-1, 0)，向右(1, 0)和向下(0, -1)
 */
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

	/**
	 * glViewport函数用于设置视口
	 * 
	 * 前两个参数设定左下角的位置
	 * 后两个参数需与实际窗口大小相同
	 */
	glViewport(0, 0, 500 * 1.6, 900); 

	glUniform1i(locxsize, xsize);
	glUniform1i(locysize, ysize);

	glBindVertexArray(vao[1]);
	glDrawArrays(GL_TRIANGLES, 0, points_num); // 绘制棋盘格 (width * height * 2 个三角形)
	
	glBindVertexArray(vao[2]);
	glDrawArrays(GL_TRIANGLES, 0, 24);	 // 绘制当前方块 (8 个三角形)
	
	glBindVertexArray(next_vao);
	glDrawArrays(GL_TRIANGLES, 0, 24); // 新功能：绘制下一个方块

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

// 设定快速下落状态
void droptile() {
	isDropping = true;
}

// 键盘响应事件中的特殊按键响应
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// 如果按下P键 切换暂停状态
	if (!gameover && key == GLFW_KEY_P) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			isPaused = !isPaused;
			return;
		}
	}

	// 暂停时
	if (isPaused) {
		// 结束游戏
		if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) {
			if(action == GLFW_PRESS){
					exit(EXIT_SUCCESS); // 结束游戏
			}
		}
		// 重新启动游戏
		else if (key == GLFW_KEY_R) {
			restart(); 
		}
		return; // 除了退出游戏，重新开始游戏和恢复游戏，其他按键都无效
	}

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
					exit(EXIT_SUCCESS); // 结束游戏
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

	// 创建窗口。glfwCreateWindow传入的窗口的实际大小（为保证比例，将宽度也设置为1.6倍）
	GLFWwindow* window = glfwCreateWindow(500 * 1.6, 900, "Mid-Term-Skeleton-Code", NULL, NULL);
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
		// 非暂停情况，才自动下落
		if (!isPaused) {
			double currentTime = glfwGetTime();
			// 如果正在快速下落状态，就以dropFPS的帧率下降。若不在快速下落，就每隔1s自动下落一格
			if (currentTime - startTime >= (isDropping ? quickDropInterval : 1.0)) {
				// 对移动失败的检测，到达底部后，就安置方块并刷新，且将isDropping设置为false
				if (!movetile(glm::vec2(0, -1)) ) {
					settile();
					newtile();
					isDropping = false;
				}
				startTime = currentTime; // 自动下落触发后，更新迭代时间
			}
		}

        display();
        glfwSwapBuffers(window);
        glfwPollEvents();	
    }
    glfwTerminate();
    return 0;
}

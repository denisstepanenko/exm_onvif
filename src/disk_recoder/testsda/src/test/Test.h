/*
 * Test.h
 *
 *  Created on: 2013-10-24
 *      Author: yangkun
 */

#ifndef TEST_H_
#define TEST_H_
#include "../Blocks.h"
#include "../YearBlocks.h"
#include "../DayBlocks.h"
#include "../SecBlocks.h"
using namespace gtsda;
int TestBlocks();
int TestYearBlocks();

#define TestInMemSize (1*1024*1024*1024)
#define HdSize (TestInMemSize/512)

//‘⁄ƒ⁄¥Ê÷–≤‚ ‘∂¡–¥
class TestInMem
{
public:

	static char *get_hd(){return hd;};
	static TestInMem* newtest();

	~TestInMem();
	void start();
	int write();
	int Init();
	void format();
	void Stop(){bIsRun = false;};
private:
	TestInMem();
	YearBlocks *yb;
	DayBlocks *db;
	DayBlocks *dbbac;//day_bac_block
	SecBlocks *hb;
	static char   *hd;
	bool bMarkWR;/*false write 1 read*/
	bool bIsRun;
};
#endif /* TEST_H_ */

/*
 * hdwr.h
 *
 *  Created on: 2013-10-28
 *      Author: yangkun
 */

#ifndef HDWR_H_
#define HDWR_H_

namespace gtsda
{

#ifndef	MEMIN
#ifdef __cplusplus
extern "C" {
#endif //end of__cplusplus
int init_sda();
#endif //end of MEMIN

//Ó²ÅÌ¶ÁÐ´º¯Êý
extern int hd_read (long long llSeek, char *cBuff, unsigned int uNumOfBlocks);
extern int hd_write(long long llSeek, char *cBuff, unsigned int uNumOfBlocks);
extern long long hd_getblocks();
#ifndef	MEMIN
#ifdef __cplusplus
}
#endif//end of__cplusplus
#endif //end of MEMIN
} /* namespace gtsda */
#endif /* HDWR_H_ */

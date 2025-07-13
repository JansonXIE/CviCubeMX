#ifndef CVI_BOARD_INIT_H
#define CVI_BOARD_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize board pin multiplexing
 */
void cvi_board_init(void);

/**
 * @brief Get board initialization status
 * @return Board initialization status (0: Success, other: Error)
 */
int cvi_board_init_status(void);

#ifdef __cplusplus
}
#endif

#endif // CVI_BOARD_INIT_H

#ifndef __PATTERN_RENDERER_H__
#define __PATTERN_RENDERER_H__

typedef struct _rd rd_t;

/**
 * @brief Init renderer and return the handler
 * @return Pointer to rd_t
 */
rd_t *rd_init(void);

/**
 * @brief Add a header
 * @param rd Handler rd_t
 * @param text Header text
 * @param format Header format
 * Header format "<n" or ">n", while "<" means left alignment
 *   ">" means right alighment, n is the column size
 */
void rd_add_header(rd_t *rd, const char *text, const char *format);

/**
 * @brief Add a item
 * @param rd Handler rd_t
 * @param item Content
 */
void rd_add_item(rd_t *rd, const char *item);

/**
 * @brief Print the table
 * @param rd Handler rd_t
 */
int rd_print(rd_t *rd);

/**
 * @brief Close the renderer, release the resource
 * @param rd Handler rd_t
 */
void rd_close(rd_t *rd);

#endif

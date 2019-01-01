#include "alist.h"

#define NULL 0

/** 初始化链表plist */
void init_list(struct list* plist) {
    plist->head.pre_elem = NULL;
    plist->head.nxt_elem = &(plist->tail);
    plist->tail.pre_elem = &(plist->head);
    plist->tail.nxt_elem = NULL;
    plist->size = 0;
}

/** 插入结点到before之前 */
void insert_list(struct list_elem* before, struct list_elem* elem) {
    struct list_elem* temp = before->pre_elem;
    temp->nxt_elem = elem;
    elem->pre_elem = temp;
    elem->nxt_elem = before;
    before->pre_elem = elem;
}

/** 将elem推入到plist最后 */
void push_list(struct list* plist, struct list_elem* elem) {
    insert_list(&(plist->tail), elem);
    plist->size++;
}

/** 在plist中移除re_elem, 其中plist参数主要是因为要更新size */
void remove_elem(struct list* plist, struct list_elem* re_elem) {
    re_elem->pre_elem->nxt_elem = re_elem->nxt_elem;
    re_elem->nxt_elem->pre_elem = re_elem->pre_elem;
    plist->size--;
}

/** 判断plist是否为空，有两种方法。 */
bool list_empty(struct list* plist) {
    /** if(plist->head->nxt_elem == plist->tail && plist->tail->pre_elem == plist->head) 
      *     return true;
      * return false; */
    return plist->size == 0;
}

/** 链表遍历函数 
 *  fn为自定义的回调函数
 * */
struct list_elem* list_traversal(struct list* plist, bool(*fn)(struct list_elem*, int),  int arg) {
    if(list_empty(plist)) {
        return NULL;
    }
    struct list_elem* temp = plist->head.nxt_elem;
    while(temp != &(plist->tail)) {
        if(fn(temp, arg)) {
            return temp;
        }
        temp = temp->nxt_elem;
    }
    return NULL;
}

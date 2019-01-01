#ifndef ALIST
#define ALIST 

#define bool int
#define true 1
#define false 0

struct list_elem {
    struct list_elem* pre_elem;
    struct list_elem* nxt_elem;
};

struct list {
    struct list_elem head;		//为什么不用指针是有讲究的
    struct list_elem tail;
    int    size; // for debug
};
/** 初始化链表plist */
void init_list(struct list* plist);

/** 将elem推入到plist最后 */
void push_list(struct list* plist, struct list_elem* elem);

/** 在plist中移除re_elem, 其中plist参数主要是因为要更新size */
void remove_elem(struct list* plist, struct list_elem* re_elem);

/** 判断plist是否为空，有两种方法。 */
bool list_empty(struct list* plist);

/** 链表遍历函数 
 *  fn为自定义的回调函数 */
struct list_elem* list_traversal(struct list* plist, bool(*fn)(struct list_elem*, int), int arg);

#endif /* ifndef ALIST */

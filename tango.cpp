#include <cstdio>
#include <cstdlib>
#include <cassert>

//Reference tree node
struct RefNode {
    int key;
    RefNode *left, *right, *parent;
    void *aux_ptr;

    // preferred child
    RefNode *preferred;

    RefNode(int k) : key(k), left(nullptr), right(nullptr), parent(nullptr),
                     aux_ptr(nullptr), preferred(nullptr) {}
};

//Auxiliary (splay) tree node
struct AuxNode {
    RefNode *ref;
    AuxNode *left, *right, *parent;
    AuxNode(RefNode *r) : ref(r), left(nullptr), right(nullptr), parent(nullptr) {}
};

//Splay helpers
void aux_set_auxptr(AuxNode *a, RefNode *r) {
    if (r) r->aux_ptr = a;
}

void rotate_right(AuxNode *x) {
    AuxNode *p = x->parent;
    if (!p) return;
    AuxNode *g = p->parent;

    p->left = x->right;
    if (x->right) x->right->parent = p;
    x->right = p;
    p->parent = x;

    x->parent = g;
    if (g) {
        if (g->left == p) g->left = x;
        else g->right = x;
    }
}

void rotate_left(AuxNode *x) {
    AuxNode *p = x->parent;
    if (!p) return;
    AuxNode *g = p->parent;

    p->right = x->left;
    if (x->left) x->left->parent = p;
    x->left = p;
    p->parent = x;

    x->parent = g;
    if (g) {
        if (g->left == p) g->left = x;
        else g->right = x;
    }
}

AuxNode* splay(AuxNode *x) {
    if (!x) return nullptr;
    while (x->parent) {
        AuxNode *p = x->parent;
        AuxNode *g = p->parent;
        if (!g) {
            // zig
            if (p->left == x) rotate_right(x);
            else rotate_left(x);
        } else if (g->left == p && p->left == x) {
            // zig-zig
            rotate_right(p);
            rotate_right(x);
        } else if (g->right == p && p->right == x) {
            // zig-zig
            rotate_left(p);
            rotate_left(x);
        } else if (g->left == p && p->right == x) {
            // zig-zag
            rotate_left(x);
            rotate_right(x);
        } else {
            // zig-zag
            rotate_right(x);
            rotate_left(x);
        }
    }
    return x;
}

// --- New aux utilities: find min/max, set aux_ptr across subtree ---
AuxNode* aux_find_min(AuxNode *r) {
    if (!r) return nullptr;
    while (r->left) r = r->left;
    return r;
}
AuxNode* aux_find_max(AuxNode *r) {
    if (!r) return nullptr;
    while (r->right) r = r->right;
    return r;
}

void aux_set_all_auxptr(AuxNode *a_root) {
    if (!a_root) return;
    struct Stack { AuxNode* n; Stack* next; Stack(AuxNode* x):n(x),next(nullptr){} };
    Stack *st = new Stack(a_root);
    while (st) {
        Stack *t = st;
        AuxNode *n = t->n;
        st = st->next;
        n->ref->aux_ptr = n;
        if (n->left) { Stack *s = new Stack(n->left); s->next = st; st = s; }
        if (n->right) { Stack *s = new Stack(n->right); s->next = st; st = s; }
        delete t;
    }
}

// --- Aux split by key and aux merge ---
// Splits 'root' into left and right where left has keys <= key, right has keys > key
void aux_split_by_key(AuxNode *root, int key, AuxNode *&left, AuxNode *&right) {
    left = right = nullptr;
    if (!root) return;
    // Find node with largest key <= key (candidate). Traverse like BST keeping candidate.
    AuxNode *cur = root;
    AuxNode *candidate = nullptr;
    while (cur) {
        if (cur->ref->key <= key) {
            candidate = cur;
            cur = cur->right;
        } else {
            cur = cur->left;
        }
    }
    if (!candidate) {
        // all nodes > key => left = nullptr, right = root (splay min to root for locality)
        AuxNode *minn = aux_find_min(root);
        root = splay(minn);
        right = root;
        if (right) {
            right->parent = nullptr;
        }
        left = nullptr;
    } else {
        // splay candidate to root
        root = splay(candidate);
        left = root;
        right = root->right;
        if (right) {
            right->parent = nullptr;
        }
        left->right = nullptr;
    }
    // set aux_ptrs properly
    aux_set_all_auxptr(left);
    aux_set_all_auxptr(right);
}

// Merge two aux trees: all keys in left <= keys in right
AuxNode* aux_merge(AuxNode *left, AuxNode *right) {
    if (!left) {
        aux_set_all_auxptr(right);
        return right;
    }
    if (!right) {
        aux_set_all_auxptr(left);
        return left;
    }
    // splay max of left to root
    AuxNode *m = aux_find_max(left);
    left = splay(m);
    // attach right
    left->right = right;
    right->parent = left;
    aux_set_all_auxptr(left);
    return left;
}

// Build splay tree from array using iterative merges (demonstrates merge usage)
AuxNode* build_splay_from_array_with_merge(RefNode **arr, int l, int r) {
    AuxNode *root = nullptr;
    for (int i = l; i <= r; ++i) {
        AuxNode *node = new AuxNode(arr[i]);
        node->left = node->right = node->parent = nullptr;
        // merge existing root with single node (arr[i])
        root = aux_merge(root, node);
    }
    return root;
}

// In-order traversal of splay
void print_aux_inorder(AuxNode *a) {
    if (!a) return;
    print_aux_inorder(a->left);
    printf("%d ", a->ref->key);
    print_aux_inorder(a->right);
}

// Free an aux tree
void free_aux_tree(AuxNode *a) {
    if (!a) return;
    free_aux_tree(a->left);
    free_aux_tree(a->right);
    a->ref->aux_ptr = nullptr;
    delete a;
}

//Reference tree helpers
RefNode* build_ref_from_sorted(int *arr, int l, int r) {
    if (l > r) return nullptr;
    int mid = (l + r) / 2;
    RefNode* node = new RefNode(arr[mid]);
    node->left = build_ref_from_sorted(arr, l, mid-1);
    if (node->left) node->left->parent = node;
    node->right = build_ref_from_sorted(arr, mid+1, r);
    if (node->right) node->right->parent = node;
    node->preferred = nullptr;
    node->aux_ptr = nullptr;
    return node;
}

RefNode* bst_search(RefNode *root, int key) {
    RefNode *cur = root;
    while (cur) {
        if (key == cur->key) return cur;
        if (key < cur->key) cur = cur->left;
        else cur = cur->right;
    }
    return nullptr;
}

// BST insert (no rebalancing)
RefNode* bst_insert(RefNode *&root, int key) {
    if (!root) {
        root = new RefNode(key);
        return root;
    }
    RefNode *cur = root;
    RefNode *par = nullptr;
    while (cur) {
        par = cur;
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return cur;
    }
    RefNode *n = new RefNode(key);
    n->parent = par;
    if (key < par->key) par->left = n;
    else par->right = n;
    return n;
}

// BST transplant for delete
void bst_transplant(RefNode *&root, RefNode *u, RefNode *v) {
    if (!u->parent) root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}

// Find min in subtree
RefNode* bst_minimum(RefNode* x) {
    while (x && x->left) x = x->left;
    return x;
}

// BST delete node
void bst_delete(RefNode *&root, RefNode *z) {
    if (!z) return;
    if (z->left == nullptr) {
        bst_transplant(root, z, z->right);
    } else if (z->right == nullptr) {
        bst_transplant(root, z, z->left);
    } else {
        RefNode *y = bst_minimum(z->right);
        if (y->parent != z) {
            bst_transplant(root, y, y->right);
            y->right = z->right;
            if (y->right) y->right->parent = y;
        }
        bst_transplant(root, z, y);
        y->left = z->left;
        if (y->left) y->left->parent = y;
    }
    // free z
    delete z;
}

// Collect the root→target path, return length
int collect_path(RefNode *root, RefNode *target, RefNode **out_arr, int maxn) {
    int idx = 0;
    RefNode *cur = root;
    while (cur && idx < maxn) {
        out_arr[idx++] = cur;
        if (target->key == cur->key) break;
        if (target->key < cur->key) cur = cur->left;
        else cur = cur->right;
    }
    return idx;
}

//update preferred pointers
void set_preferred_along_path(RefNode *root, RefNode **path, int len) {
    struct ClearStack { RefNode* n; ClearStack* next; ClearStack(RefNode* x):n(x),next(nullptr){} };
    ClearStack *stack = nullptr;
    if (root) {
        stack = new ClearStack(root);
    }
    while (stack) {
        ClearStack *top = stack;
        RefNode *n = top->n;
        stack = stack->next;
        n->preferred = nullptr;
        if (n->left) {
            ClearStack *s = new ClearStack(n->left);
            s->next = stack; stack = s;
        }
        if (n->right) {
            ClearStack *s = new ClearStack(n->right);
            s->next = stack; stack = s;
        }
        delete top;
    }

    //Set preferred pointers along the path
    for (int i = 0; i + 1 < len; ++i) {
        path[i]->preferred = path[i+1];
    }
    if (len > 0) path[len-1]->preferred = nullptr;
}

struct AuxListNode { AuxNode* aroot; AuxListNode* next; AuxListNode(AuxNode* r): aroot(r), next(nullptr){} };

AuxListNode* build_aux_trees_from_ref(RefNode *root) {
    if (!root) return nullptr;
    struct Stack { RefNode* n; Stack* next; Stack(RefNode* x):n(x),next(nullptr){} };
    Stack *st = new Stack(root);
    AuxListNode *alist = nullptr;

    while (st) {
        Stack *t = st;
        RefNode *n = t->n;
        st = st->next;
        if (n->right) { Stack *s = new Stack(n->right); s->next = st; st = s; }
        if (n->left)  { Stack *s = new Stack(n->left);  s->next = st; st = s; }
        if (!n->parent || n->parent->preferred != n) {
            int maxlen = 1024; // initial; if path longer, we'll reallocate
            RefNode **arr = (RefNode**)malloc(sizeof(RefNode*) * maxlen);
            int len = 0;
            RefNode *cur = n;
            while (cur) {
                if (len >= maxlen) {
                    maxlen *= 2;
                    arr = (RefNode**)realloc(arr, sizeof(RefNode*) * maxlen);
                }
                arr[len++] = cur;
                cur = cur->preferred;
            }
            // Build splay from arr[0..len-1] using merge-based builder
            AuxNode *aroot = build_splay_from_array_with_merge(arr, 0, len-1);
            AuxListNode *an = new AuxListNode(aroot);
            an->next = alist; alist = an;
            free(arr);
        }
        delete t;
    }
    return alist;
}

// Free auxiliary list
void free_aux_list(AuxListNode *alist) {
    AuxListNode *cur = alist;
    while (cur) {
        free_aux_tree(cur->aroot);
        AuxListNode *n = cur->next;
        delete cur;
        cur = n;
    }
}

//Tango structure
struct Tango {
    RefNode *ref_root;
    AuxListNode *aux_list;

    Tango(): ref_root(nullptr), aux_list(nullptr) {}

    void build_from_sorted_array(int *arr, int n) {
        ref_root = build_ref_from_sorted(arr, 0, n-1);
        // initially no preferred pointers
        rebuild_aux();
    }

    void rebuild_aux() {
        // free previous aux trees
        if (aux_list) {
            free_aux_list(aux_list);
            aux_list = nullptr;
        }
        aux_list = build_aux_trees_from_ref(ref_root);
    }

    // Access operation (Search): find node and update preferred path.
    RefNode* access(int key) {
        RefNode *target = bst_search(ref_root, key);
        if (!target) {
            return nullptr;
        }
        const int MAXP = 100000;
        RefNode **path = (RefNode**)malloc(sizeof(RefNode*) * 1000);
        int capacity = 1000;
        int len = 0;
        RefNode *cur = ref_root;
        while (cur) {
            if (len >= capacity) {
                capacity *= 2;
                path = (RefNode**)realloc(path, sizeof(RefNode*) * capacity);
            }
            path[len++] = cur;
            if (cur->key == key) break;
            if (key < cur->key) cur = cur->left;
            else cur = cur->right;
        }
        // set preferred pointers along path
        set_preferred_along_path(ref_root, path, len);
        free(path);

        // Rebuild auxiliary trees for new preferred decomposition
        // NOTE: This is a full rebuild. With aux_split/aux_merge you can implement incremental update here.
        rebuild_aux();
        return target;
    }

    // Insert key into reference tree, then rebuild aux
    void insert_key(int key) {
        RefNode *n = bst_insert(ref_root, key);
        (void)n;
        rebuild_aux();
    }

    // Remove key
    void remove_key(int key) {
        RefNode *z = bst_search(ref_root, key);
        if (!z) return;
        bst_delete(ref_root, z);
        rebuild_aux();
    }

    void print_ref_inorder(RefNode *r) {
        if (!r) return;
        print_ref_inorder(r->left);
        printf("%d ", r->key);
        print_ref_inorder(r->right);
    }
    void print_ref_tree() { print_ref_inorder(ref_root); printf("\n"); }

    void print_aux_trees() {
        printf("Aux trees (roots):\n");
        AuxListNode *cur = aux_list;
        int idx = 0;
        while (cur) {
            printf("Aux %d: ", idx++);
            print_aux_inorder(cur->aroot);
            printf("\n");
            cur = cur->next;
        }
    }

private:
    void print_ref_inorder(RefNode *r, int depth) {
        (void)depth;
        if (!r) return;
        print_ref_inorder(r->left, depth+1);
        printf("%d ", r->key);
        print_ref_inorder(r->right, depth+1);
    }
};

int main() {
    // Build Tango from sorted keys
    int keys[] = {10, 20, 30, 40, 50, 60, 70};
    int n = sizeof(keys)/sizeof(keys[0]);

    Tango T;
    T.build_from_sorted_array(keys, n);

    printf("Initial reference tree inorder: ");
    T.print_ref_tree();
    T.print_aux_trees();

    printf("\nAccess 50\n");
    T.access(50);
    T.print_aux_trees();

    printf("\nAccess 20\n");
    T.access(20);
    T.print_aux_trees();

    printf("\nInsert 25\n");
    T.insert_key(25);
    T.print_ref_tree();
    T.print_aux_trees();

    printf("\nAccess 25\n");
    T.access(25);
    T.print_aux_trees();

    printf("\nRemove 40\n");
    T.remove_key(40);
    T.print_ref_tree();
    T.print_aux_trees();

    return 0;
}

/*
#include <cstdio>
#include <cstdlib>
#include <cassert>

//Reference tree node
struct RefNode {
    int key;
    RefNode *left, *right, *parent;
    void *aux_ptr;

    // preferred child
    RefNode *preferred;

    RefNode(int k) : key(k), left(nullptr), right(nullptr), parent(nullptr),
                     aux_ptr(nullptr), preferred(nullptr) {}
};

//Auxiliary (splay) tree node
struct AuxNode {
    RefNode *ref;    
    AuxNode *left, *right, *parent;
    AuxNode(RefNode *r) : ref(r), left(nullptr), right(nullptr), parent(nullptr) {}
};

//Splay helpers
void aux_set_auxptr(AuxNode *a, RefNode *r) {
    if (r) r->aux_ptr = a;
}

void rotate_right(AuxNode *x) {
    AuxNode *p = x->parent;
    if (!p) return;
    AuxNode *g = p->parent;

    p->left = x->right;
    if (x->right) x->right->parent = p;
    x->right = p;
    p->parent = x;

    x->parent = g;
    if (g) {
        if (g->left == p) g->left = x;
        else g->right = x;
    }
}

void rotate_left(AuxNode *x) {
    AuxNode *p = x->parent;
    if (!p) return;
    AuxNode *g = p->parent;

    p->right = x->left;
    if (x->left) x->left->parent = p;
    x->left = p;
    p->parent = x;

    x->parent = g;
    if (g) {
        if (g->left == p) g->left = x;
        else g->right = x;
    }
}

AuxNode* splay(AuxNode *x) {
    if (!x) return nullptr;
    while (x->parent) {
        AuxNode *p = x->parent;
        AuxNode *g = p->parent;
        if (!g) {
            // zig
            if (p->left == x) rotate_right(x);
            else rotate_left(x);
        } else if (g->left == p && p->left == x) {
            // zig-zig
            rotate_right(p);
            rotate_right(x);
        } else if (g->right == p && p->right == x) {
            // zig-zig
            rotate_left(p);
            rotate_left(x);
        } else if (g->left == p && p->right == x) {
            // zig-zag
            rotate_left(x);
            rotate_right(x);
        } else {
            // zig-zag
            rotate_right(x);
            rotate_left(x);
        }
    }
    return x;
}

// Build splay tree
AuxNode* build_splay_from_array(RefNode **arr, int l, int r) {
    if (l > r) return nullptr;
    int mid = (l + r) / 2;
    AuxNode *root = new AuxNode(arr[mid]);
    root->left = build_splay_from_array(arr, l, mid-1);
    if (root->left) root->left->parent = root;
    root->right = build_splay_from_array(arr, mid+1, r);
    if (root->right) root->right->parent = root;
    // set aux_ptr for ref
    root->ref->aux_ptr = root;
    return root;
}

// In-order traversal of splay
void print_aux_inorder(AuxNode *a) {
    if (!a) return;
    print_aux_inorder(a->left);
    printf("%d ", a->ref->key);
    print_aux_inorder(a->right);
}

// Free an aux tree
void free_aux_tree(AuxNode *a) {
    if (!a) return;
    free_aux_tree(a->left);
    free_aux_tree(a->right);
    a->ref->aux_ptr = nullptr;
    delete a;
}

//Reference tree helpers
RefNode* build_ref_from_sorted(int *arr, int l, int r) {
    if (l > r) return nullptr;
    int mid = (l + r) / 2;
    RefNode* node = new RefNode(arr[mid]);
    node->left = build_ref_from_sorted(arr, l, mid-1);
    if (node->left) node->left->parent = node;
    node->right = build_ref_from_sorted(arr, mid+1, r);
    if (node->right) node->right->parent = node;
    node->preferred = nullptr;
    node->aux_ptr = nullptr;
    return node;
}

RefNode* bst_search(RefNode *root, int key) {
    RefNode *cur = root;
    while (cur) {
        if (key == cur->key) return cur;
        if (key < cur->key) cur = cur->left;
        else cur = cur->right;
    }
    return nullptr;
}

// BST insert (no rebalancing)
RefNode* bst_insert(RefNode *&root, int key) {
    if (!root) {
        root = new RefNode(key);
        return root;
    }
    RefNode *cur = root;
    RefNode *par = nullptr;
    while (cur) {
        par = cur;
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return cur; 
    }
    RefNode *n = new RefNode(key);
    n->parent = par;
    if (key < par->key) par->left = n;
    else par->right = n;
    return n;
}

// BST transplant for delete
void bst_transplant(RefNode *&root, RefNode *u, RefNode *v) {
    if (!u->parent) root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}

// Find min in subtree
RefNode* bst_minimum(RefNode* x) {
    while (x && x->left) x = x->left;
    return x;
}

// BST delete node
void bst_delete(RefNode *&root, RefNode *z) {
    if (!z) return;
    if (z->left == nullptr) {
        bst_transplant(root, z, z->right);
    } else if (z->right == nullptr) {
        bst_transplant(root, z, z->left);
    } else {
        RefNode *y = bst_minimum(z->right);
        if (y->parent != z) {
            bst_transplant(root, y, y->right);
            y->right = z->right;
            if (y->right) y->right->parent = y;
        }
        bst_transplant(root, z, y);
        y->left = z->left;
        if (y->left) y->left->parent = y;
    }
    // free z
    delete z;
}

// Collect the root→target path, return length
int collect_path(RefNode *root, RefNode *target, RefNode **out_arr, int maxn) {
    int idx = 0;
    RefNode *cur = root;
    while (cur && idx < maxn) {
        out_arr[idx++] = cur;
        if (target->key == cur->key) break;
        if (target->key < cur->key) cur = cur->left;
        else cur = cur->right;
    }
    return idx;
}

//update preferred pointers
void set_preferred_along_path(RefNode *root, RefNode **path, int len) {
    struct ClearStack { RefNode* n; ClearStack* next; ClearStack(RefNode* x):n(x),next(nullptr){} };
    ClearStack *stack = nullptr;
    if (root) {
        stack = new ClearStack(root);
    }
    while (stack) {
        ClearStack *top = stack;
        RefNode *n = top->n;
        stack = stack->next;
        n->preferred = nullptr;
        if (n->left) {
            ClearStack *s = new ClearStack(n->left);
            s->next = stack; stack = s;
        }
        if (n->right) {
            ClearStack *s = new ClearStack(n->right);
            s->next = stack; stack = s;
        }
        delete top;
    }

    //Set preferred pointers along the path
    for (int i = 0; i + 1 < len; ++i) {
        path[i]->preferred = path[i+1];
    }
    if (len > 0) path[len-1]->preferred = nullptr;
}

struct AuxListNode { AuxNode* aroot; AuxListNode* next; AuxListNode(AuxNode* r): aroot(r), next(nullptr){} };

AuxListNode* build_aux_trees_from_ref(RefNode *root) {
    if (!root) return nullptr;
    struct Stack { RefNode* n; Stack* next; Stack(RefNode* x):n(x),next(nullptr){} };
    Stack *st = new Stack(root);
    AuxListNode *alist = nullptr;

    while (st) {
        Stack *t = st;
        RefNode *n = t->n;
        st = st->next;
        if (n->right) { Stack *s = new Stack(n->right); s->next = st; st = s; }
        if (n->left)  { Stack *s = new Stack(n->left);  s->next = st; st = s; }
        if (!n->parent || n->parent->preferred != n) {
            int maxlen = 1024; // initial; if path longer, we'll reallocate
            RefNode **arr = (RefNode**)malloc(sizeof(RefNode*) * maxlen);
            int len = 0;
            RefNode *cur = n;
            while (cur) {
                if (len >= maxlen) {
                    maxlen *= 2;
                    arr = (RefNode**)realloc(arr, sizeof(RefNode*) * maxlen);
                }
                arr[len++] = cur;
                cur = cur->preferred;
            }
            // Build splay from arr[0..len-1]
            AuxNode *aroot = build_splay_from_array(arr, 0, len-1);
            AuxListNode *an = new AuxListNode(aroot);
            an->next = alist; alist = an;
            free(arr);
        }
        delete t;
    }
    return alist;
}

// Free auxiliary list
void free_aux_list(AuxListNode *alist) {
    AuxListNode *cur = alist;
    while (cur) {
        free_aux_tree(cur->aroot);
        AuxListNode *n = cur->next;
        delete cur;
        cur = n;
    }
}

//Tango structure
struct Tango {
    RefNode *ref_root;
    AuxListNode *aux_list;

    Tango(): ref_root(nullptr), aux_list(nullptr) {}

    void build_from_sorted_array(int *arr, int n) {
        ref_root = build_ref_from_sorted(arr, 0, n-1);
        // initially no preferred pointers
        rebuild_aux();
    }

    void rebuild_aux() {
        // free previous aux trees
        if (aux_list) {
            free_aux_list(aux_list);
            aux_list = nullptr;
        }
        aux_list = build_aux_trees_from_ref(ref_root);
    }

    // Access operation (Search): find node and update preferred path.
    RefNode* access(int key) {
        RefNode *target = bst_search(ref_root, key);
        if (!target) {
            return nullptr;
        }
        const int MAXP = 100000;
        RefNode **path = (RefNode**)malloc(sizeof(RefNode*) * 1000); 
        int capacity = 1000;
        int len = 0;
        RefNode *cur = ref_root;
        while (cur) {
            if (len >= capacity) {
                capacity *= 2;
                path = (RefNode**)realloc(path, sizeof(RefNode*) * capacity);
            }
            path[len++] = cur;
            if (cur->key == key) break;
            if (key < cur->key) cur = cur->left;
            else cur = cur->right;
        }
        // set preferred pointers along path
        set_preferred_along_path(ref_root, path, len);
        free(path);

        // Rebuild auxiliary trees for new preferred decomposition
        rebuild_aux();
        return target;
    }

    // Insert key into reference tree, then rebuild aux
    void insert_key(int key) {
        RefNode *n = bst_insert(ref_root, key);
        (void)n;
        rebuild_aux();
    }

    // Remove key
    void remove_key(int key) {
        RefNode *z = bst_search(ref_root, key);
        if (!z) return;
        bst_delete(ref_root, z);
        rebuild_aux();
    }

    void print_ref_inorder(RefNode *r) {
        if (!r) return;
        print_ref_inorder(r->left);
        printf("%d ", r->key);
        print_ref_inorder(r->right);
    }
    void print_ref_tree() { print_ref_inorder(ref_root); printf("\n"); }

    void print_aux_trees() {
        printf("Aux trees (roots):\n");
        AuxListNode *cur = aux_list;
        int idx = 0;
        while (cur) {
            printf("Aux %d: ", idx++);
            print_aux_inorder(cur->aroot);
            printf("\n");
            cur = cur->next;
        }
    }

private:
    void print_ref_inorder(RefNode *r, int depth) {
        (void)depth;
        if (!r) return;
        print_ref_inorder(r->left, depth+1);
        printf("%d ", r->key);
        print_ref_inorder(r->right, depth+1);
    }
};

int main() {
    // Build Tango from sorted keys
    int keys[] = {10, 20, 30, 40, 50, 60, 70};
    int n = sizeof(keys)/sizeof(keys[0]);

    Tango T;
    T.build_from_sorted_array(keys, n);

    printf("Initial reference tree inorder: ");
    T.print_ref_tree();
    T.print_aux_trees();

    printf("\nAccess 50\n");
    T.access(50);
    T.print_aux_trees();

    printf("\nAccess 20\n");
    T.access(20);
    T.print_aux_trees();

    printf("\nInsert 25\n");
    T.insert_key(25);
    T.print_ref_tree();
    T.print_aux_trees();

    printf("\nAccess 25\n");
    T.access(25);
    T.print_aux_trees();

    printf("\nRemove 40\n");
    T.remove_key(40);
    T.print_ref_tree();
    T.print_aux_trees();

    return 0;
}
*/
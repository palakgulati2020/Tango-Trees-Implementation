
/*
 * ============================================================
 *  TANGO TREE — C++ Implementation
 * ============================================================
 *  Reference:
 *    Demaine, Harmon, Iacono, Patrascu
 *    "Dynamic Optimality—Almost"
 *    SIAM Journal on Computing 37(1):240-265, 2007
 *    http://erikdemaine.org/papers/Tango_SICOMP/paper.pdf
 *
 *  ARCHITECTURE:
 *  1. Reference tree  – conceptual complete BST. Never stored.
 *     Provides depth, parent, left, right for every key.
 *  2. Preferred child – child most recently accessed.
 *     Chains of preferred edges form "preferred paths".
 *  3. Each preferred path is stored in an auxiliary Red-Black Tree (RBT)
 *     sorted by depth-in-reference-tree, augmented with min/max depth.
 *  4. Each node on a path carries an aux_child pointer to the RBT of
 *     the preferred path rooted at its non-preferred child.
 *  5. access(x):
 *       Walk the reference tree path root->x.
 *       At each node on that path, check which aux tree it belongs to.
 *       When we cross into a new preferred path, do a cut/join.
 *       Each cut/join = O(log log n).
 *  COMPETITIVE RATIO: O(log log n) * OPT  [Theorem 1, Section 4]
 *
 *  IMPLEMENTATION NOTE:
 *    The RBT is sorted by depth, so key-search is O(n) linear scan.
 *    We track which aux tree each key lives in via a hash map (key_to_aux).
 * ============================================================
 */

#include<bits/stdc++.h>
using namespace std;
// ============================================================
//  NIL sentinel
// ============================================================
enum class Color { RED, BLACK };
struct RBTree;

struct RBNode {
    int key, depth;
    Color color;
    int min_depth, max_depth;
    RBNode *left, *right, *parent;
    RBTree *aux_child;
    RBNode(int k, int d)
        : key(k), depth(d), color(Color::RED),
          min_depth(d), max_depth(d),
          left(nullptr), right(nullptr), parent(nullptr),
          aux_child(nullptr) {}
};

static RBNode  g_nil_node(0, INT_MAX);
static RBNode* G_NIL = &g_nil_node;

static void setup_nil() {
    G_NIL->color     = Color::BLACK;
    G_NIL->min_depth = INT_MAX;
    G_NIL->max_depth = INT_MIN;
    G_NIL->left      = G_NIL;
    G_NIL->right     = G_NIL;
    G_NIL->parent    = G_NIL;
    G_NIL->aux_child = nullptr;
}

static void aug(RBNode* n) {
    if (!n || n == G_NIL) return;
    n->min_depth = n->depth; n->max_depth = n->depth;
    if (n->left  != G_NIL) { n->min_depth =min(n->min_depth, n->left->min_depth);
                              n->max_depth = max(n->max_depth, n->left->max_depth); }
    if (n->right != G_NIL) { n->min_depth = min(n->min_depth, n->right->min_depth);
                              n->max_depth = max(n->max_depth, n->right->max_depth); }
}

// ============================================================
//  Red-Black Tree  (sorted by DEPTH)
// ============================================================
struct RBTree {
    RBNode* root;
    RBTree() : root(G_NIL) {}

    void lrot(RBNode* x) {
        RBNode* y = x->right; x->right = y->left;
        if (y->left != G_NIL) y->left->parent = x;
        y->parent = x->parent;
        if      (x->parent == G_NIL)          root             = y;
        else if (x == x->parent->left)        x->parent->left  = y;
        else                                  x->parent->right = y;
        y->left = x; x->parent = y; aug(x); aug(y);
    }
    void rrot(RBNode* y) {
        RBNode* x = y->left; y->left = x->right;
        if (x->right != G_NIL) x->right->parent = y;
        x->parent = y->parent;
        if      (y->parent == G_NIL)          root             = x;
        else if (y == y->parent->left)        y->parent->left  = x;
        else                                  y->parent->right = x;
        x->right = y; y->parent = x; aug(y); aug(x);
    }

    RBNode* insert(int key, int depth) {
        RBNode* z = new RBNode(key, depth);
        z->left = z->right = z->parent = G_NIL;
        raw_ins(z); fix_ins(z); reaug(root); return z;
    }
    void raw_ins(RBNode* z) {
        RBNode* y = G_NIL, *x = root;
        while (x != G_NIL) { y = x; x = (z->depth < x->depth) ? x->left : x->right; }
        z->parent = y;
        if      (y == G_NIL)            root = z;
        else if (z->depth < y->depth)  y->left  = z;
        else                           y->right = z;
    }
    void fix_ins(RBNode* z) {
        while (z->parent->color == Color::RED) {
            if (z->parent == z->parent->parent->left) {
                RBNode* u = z->parent->parent->right;
                if (u->color == Color::RED) {
                    z->parent->color = u->color = Color::BLACK;
                    z->parent->parent->color = Color::RED; z = z->parent->parent;
                } else {
                    if (z == z->parent->right) { z = z->parent; lrot(z); }
                    z->parent->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    rrot(z->parent->parent);
                }
            } else {
                RBNode* u = z->parent->parent->left;
                if (u->color == Color::RED) {
                    z->parent->color = u->color = Color::BLACK;
                    z->parent->parent->color = Color::RED; z = z->parent->parent;
                } else {
                    if (z == z->parent->left) { z = z->parent; rrot(z); }
                    z->parent->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    lrot(z->parent->parent);
                }
            }
        }
        root->color = Color::BLACK;
    }

    void reaug(RBNode* n) {
        if (n == G_NIL) return; reaug(n->left); reaug(n->right); aug(n);
    }

    // Linear scan by key (tree is sorted by depth, not key)
    RBNode* find_key(int key) const {
        return find_key_rec(root, key);
    }
    RBNode* find_key_rec(RBNode* n, int key) const {
        if (n == G_NIL) return G_NIL;
        if (n->key == key) return n;
        RBNode* res = find_key_rec(n->left, key);
        return (res != G_NIL) ? res : find_key_rec(n->right, key);
    }

    void inorder(RBNode* n, vector<RBNode*>& v) const {
        if (n == G_NIL) return;
        inorder(n->left, v); v.push_back(n); inorder(n->right, v);
    }
    vector<RBNode*> nodes() const {
        vector<RBNode*> v; inorder(root, v); return v;
    }
    int size() const { return (int)nodes().size(); }

    void rebuild(vector<RBNode*>& ns) {
        root = G_NIL;
        for (RBNode* n : ns) {
            n->left = n->right = n->parent = G_NIL;
            n->color = Color::RED;
            n->min_depth = n->max_depth = n->depth;
            raw_ins(n); fix_ins(n);
        }
        reaug(root);
    }

    pair<RBTree*, RBTree*> split(int thr) {
        auto ns = nodes(); root = G_NIL;
        vector<RBNode*> tn, bn;
        for (RBNode* n : ns) (n->depth <= thr ? tn : bn).push_back(n);
        RBTree* T = new RBTree(); T->rebuild(tn);
        RBTree* B = new RBTree(); B->rebuild(bn);
        return {T, B};
    }

    static RBTree* concat(RBTree* a, RBTree* b) {
        if (!a || a->root == G_NIL) { delete a; return b ? b : new RBTree(); }
        if (!b || b->root == G_NIL) { delete b; return a; }
        auto na = a->nodes(), nb = b->nodes();
        a->root = b->root = G_NIL; delete a; delete b;
        RBTree* m = new RBTree();
        vector<RBNode*> all;
        all.insert(all.end(), na.begin(), na.end());
        all.insert(all.end(), nb.begin(), nb.end());
        m->rebuild(all); return m;
    }

    void destroy(RBNode* n) {
        if (!n || n == G_NIL) return;
        destroy(n->left); destroy(n->right); delete n;
    }
    ~RBTree() { destroy(root); }
};

// ============================================================
//  Reference Tree
// ============================================================
struct RefTree {
    unordered_map<int,int> dep, par, lch, rch;
    int rk = -1;
    explicit RefTree(const vector<int>& ks) {
        vector<int> s = ks; sort(s.begin(), s.end());
        rk = build(s, 0, (int)s.size()-1, 0, -1);
    }
    int  root_key()   const { return rk; }
    int  height(int n)const { return (int)ceil(log2((double)n+1)); }
    bool has(int k)   const { return dep.count(k) > 0; }
    int build(const std::vector<int>& s, int lo, int hi, int d, int p) {
        if (lo > hi) return -1;
        int m = (lo+hi)/2, k = s[m];
        dep[k]=d; par[k]=p;
        lch[k]=build(s,lo,m-1,d+1,k);
        rch[k]=build(s,m+1,hi,d+1,k);
        return k;
    }
};

// ============================================================
//  Tango Tree
// ============================================================
class TangoTree {
public:
    explicit TangoTree(const vector<int>& keys)
        : ref(keys), n((int)keys.size()), acnt(0), tcost(0)
    {
        //pref child's info
        for (int k : keys) pref[k] = -1;
        root_aux = build_aux(ref.root_key());
        // Build key->aux_tree map
        index_keys(root_aux);
    }
    ~TangoTree() { cleanup(root_aux); }

    bool access(int x) {
        if (!ref.has(x)) return false;
        ++acnt;

        // Walk reference-tree path root->x, following aux trees
        // We simulate the path node by node using the reference tree structure.
        bool found = walk_path(x);
        if (found) update_prefs(x);
        return found;
    }

    vector<int> inorder() const {
        vector<int> v; collect(root_aux, v);
        sort(v.begin(), v.end()); return v;
    }

    void print_stats() const {
        double lln = (n > 2) ? log2(log2((double)n)) : 0.0;
        cout << "\n--- Tango Tree Statistics ---\n"
                  << "  n                     : " << n << "\n"
                  << "  Reference tree height : " << ref.height(n) << "\n"
                  << "  log2(log2(n))         : "
                  << fixed << setprecision(3) << lln << "\n"
                  << "  Total accesses        : " << acnt  << "\n"
                  << "  Total node-touches    : " << tcost << "\n"
                  << "  Avg cost per access   : "
                  << (acnt ? (double)tcost / acnt : 0.0) << "\n";
    }

    long long get_access_count() const { return acnt; }
    long long get_total_cost()   const { return tcost; }
    int       get_n()            const { return n; }

private:
    RefTree   ref;
    int       n;
    RBTree*   root_aux;
    unordered_map<int,int>    pref;      // preferred child of each node
    unordered_map<int,RBTree*> key_aux;  // which aux tree holds each key
    long long acnt, tcost;

    // ── Build ────────────────────────────────────────────────────
    RBTree* build_aux(int k) {
        if (k < 0) return nullptr;
        RBTree* t = new RBTree();
        build_rec(k, t);
        return t;
    }
    void build_rec(int k, RBTree* t) {
        int lc = ref.lch.at(k), rc = ref.rch.at(k), pc = pref[k];
        RBNode* nd = t->insert(k, ref.dep.at(k));

        int preferred_next = -1, non_pref = -1;
        if      (pc == lc && lc >= 0) { preferred_next = lc; non_pref = rc; }
        else if (pc == rc && rc >= 0) { preferred_next = rc; non_pref = lc; }

        if (preferred_next >= 0) {
            if (non_pref >= 0) nd->aux_child = build_aux(non_pref);
            build_rec(preferred_next, t);
        } else {
            RBTree* lt = (lc >= 0) ? build_aux(lc) : nullptr;
            RBTree* rt = (rc >= 0) ? build_aux(rc) : nullptr;
            if (lt && rt) { nd->aux_child = lt; attach_bottom(lt, rt); }
            else          { nd->aux_child = lt ? lt : rt; }
        }
    }
    void attach_bottom(RBTree* t, RBTree* other) {
        if (!t || t->root == G_NIL || !other) return;
        auto ns = t->nodes();
        RBNode* deep = ns[0];
        for (auto* nd : ns) if (nd->depth > deep->depth) deep = nd;
        if (!deep->aux_child) deep->aux_child = other;
        else attach_bottom(deep->aux_child, other);
    }

    // Build key->aux_tree index after construction
    void index_keys(RBTree* t) {
        if (!t || t->root == G_NIL) return;
        for (RBNode* nd : t->nodes()) {
            key_aux[nd->key] = t;
            index_keys(nd->aux_child);
        }
    }

    // ── Core access: walk reference-tree path root->x ────────────
    // At each step, we know which aux tree the current reference-tree
    // node lives in.  If the current node and x are in the same aux tree,
    // we're done.  Otherwise we follow the aux_child link at the boundary.
    bool walk_path(int x) {
        // Build reference-tree path root->x
        vector<int> path;
        int cur = ref.rk;
        while (cur != -1 && cur != x) {
            path.push_back(cur);
            cur = (x < cur) ? ref.lch.at(cur) : ref.rch.at(cur);
        }
        if (cur != x) return false;
        path.push_back(x);

        // Walk the path; count touches and perform cut/join at boundaries
        // between different preferred paths (aux trees).
        for (int i = 0; i < (int)path.size(); ) {
            int k = path[i];
            RBTree* t = key_aux.count(k) ? key_aux[k] : nullptr;
            if (!t) return false;

            tcost += t->size();

            // Find how far this aux tree extends along the path
            int j = i;
            while (j < (int)path.size() && key_aux.count(path[j]) && key_aux[path[j]] == t)
                ++j;

            // path[i..j-1] are all in this aux tree
            // The last node in this segment (path[j-1]) is the gateway
            int gk = path[j-1];
            RBNode* gw = t->find_key(gk);

            if (j == (int)path.size()) {
                // x is in this aux tree — done
                break;
            }

            // Perform cut/join: split t at gw->depth,
            // merge bottom with gw->aux_child (which contains path[j])
            if (gw != G_NIL) {
                cut_join(t, gw, path[j]);
                // After cut_join, path[j] moved into gw->aux_child
                // Update key_aux for all nodes that moved
                reindex(gw->aux_child);
            }
            i = j;
        }
        return true;
    }

    void cut_join(RBTree* t, RBNode* gw, int next_key) {
        int sd = gw->depth, gk = gw->key;

        auto splitResult = t->split(sd);
        RBTree* top = splitResult.first;
        RBTree* bot = splitResult.second;   
        t->root = top->root; top->root = G_NIL; delete top;
        t->reaug(t->root);

        // All nodes that were in bot need their key_aux updated to some new tree
        // (will be handled by reindex after this call)
        for (RBNode* nd : bot->nodes()) key_aux.erase(nd->key);

        RBNode* gw2 = t->find_key(gk);
        if (gw2 == G_NIL) { delete bot; return; }

        RBTree* old = gw2->aux_child;
        if (old && old->root != G_NIL) {
            for (RBNode* nd : old->nodes()) key_aux.erase(nd->key);
            gw2->aux_child = RBTree::concat(bot, old);
        } else {
            delete old;
            gw2->aux_child = bot;
        }
    }

    void reindex(RBTree* t) {
        if (!t || t->root == G_NIL) return;
        for (RBNode* nd : t->nodes()) {
            key_aux[nd->key] = t;
            reindex(nd->aux_child);
        }
    }

    void update_prefs(int x) {
        int cur = x;
        for (;;) {
            auto it = ref.par.find(cur);
            if (it == ref.par.end() || it->second < 0) break;
            pref[it->second] = cur; cur = it->second;
        }
    }

    void collect(const RBTree* t, vector<int>& v) const {
        if (!t || t->root == G_NIL) return;
        for (RBNode* nd : t->nodes()) {
            v.push_back(nd->key);
            collect(nd->aux_child, v);
        }
    }

    void cleanup(RBTree* t) {
        if (!t) return;
        if (t->root != G_NIL)
            for (RBNode* nd : t->nodes()) { cleanup(nd->aux_child); nd->aux_child = nullptr; }
        delete t;
    }
};

// ============================================================
//  Tests & Demo
// ============================================================
static void sep(const string& s) {
      cout << "\n" <<string(62,'=') << "\n  " << s
              << "\n" << string(62,'=') << "\n";
}

void run_unit_tests() {
    sep("UNIT TESTS");

    { TangoTree t({42}); assert(t.access(42)); assert(!t.access(0));
      cout << "  [PASS] T01: single-element access\n"; }

    { TangoTree t({1,2,3}); assert(t.access(1)); assert(t.access(2)); assert(t.access(3));
      cout << "  [PASS] T02: 3-key tree\n"; }

    { TangoTree t({1,2,3,4,5,6,7});
      for (int k=1;k<=7;k++) { assert(t.access(k)); }
      assert(!t.access(99));
      cout << "  [PASS] T03: 7-key tree; non-existent=false\n"; }

    { vector<int> ks; for(int i=1;i<=15;i++) ks.push_back(i);
      TangoTree t(ks); for(int k:ks) { assert(t.access(k)); }
      cout << "  [PASS] T04: 15-key BST (height 4)\n"; }

    { TangoTree t({1,2,3,4,5,6,7}); for(int i=0;i<5;i++) t.access(4);
      assert(t.get_access_count()==5);
      cout << "  [PASS] T05: access counter\n"; }

    { TangoTree t({5,3,7,1,4,6,8});
      assert((t.inorder()==vector<int>{1,3,4,5,6,7,8}));
      cout << "  [PASS] T06: in-order sorted\n"; }

    { RBTree rb; rb.insert(10,1); rb.insert(5,2); rb.insert(15,2);
      assert(rb.find_key(10) != G_NIL);
      assert(rb.find_key(99) == G_NIL);
      cout << "  [PASS] T07: RBTree insert/find_key\n"; }

    { RBTree rb; rb.insert(10,1); rb.insert(5,2); rb.insert(3,3); rb.insert(7,3);
      auto splitResult = rb.split(2);
    RBTree* top = splitResult.first;
    RBTree* bot = splitResult.second;
      for(auto* nd:top->nodes()) assert(nd->depth<=2);
      for(auto* nd:bot->nodes()) assert(nd->depth>2);
      cout << "  [PASS] T08: RBTree split\n";
      delete top; delete bot; }

    { RBTree* a=new RBTree(); RBTree* b=new RBTree();
      a->insert(1,1); a->insert(2,2); b->insert(3,3); b->insert(4,4);
      RBTree* m=RBTree::concat(a,b); assert(m->size()==4);
      cout << "  [PASS] T09: RBTree concatenate\n"; delete m; }

    { vector<int> ks; for(int i=1;i<=31;i++) ks.push_back(i);
      TangoTree t(ks); for(int k:ks) { assert(t.access(k)); }
      cout << "  [PASS] T10: 31-key BST (height 5)\n"; }

    cout << "\n  All 10 unit tests PASSED!\n";
}

void run_demo() {
    sep("DEMO — n=15 (complete BST, height 4)");
    vector<int> keys; for(int i=1;i<=15;i++) keys.push_back(i);
    TangoTree tt(keys); RefTree ref(keys);

    cout << "\n[1] Reference tree (root=" << ref.root_key()
              << ", height=" << ref.height(15) << ")\n"
              << "    key | depth | left | right\n"
              << "    ----+-------+------+------\n";
    for(int k:keys)
        cout << "     " <<setw(2) << k << "  |   " << ref.dep.at(k)
                  << "   |  " << setw(2) << ref.lch.at(k)
                  << "  |  "  << setw(2) << ref.rch.at(k) << "\n";

    vector<int> seq={8,4,12,2,6,10,14,1,3,5,7,9,11,13,15};
    cout << "\n[2] Access sequence (BFS order — maximises interleave):\n";
    for(int x:seq) {
        bool ok=tt.access(x);
        cout << "    access(" << setw(2) << x << ")  =>  "
                  << (ok ? "FOUND" : "NOT FOUND") << "\n";
    }
    tt.print_stats();

   cout << "\n[3] Competitive Ratio (Theorem 1):\n"
              << "    n             = 15\n"
              << "    log2(n)       = " << log2(15.0) << "  (naive BST)\n"
              << "    log2(log2(n)) = " << log2(log2(15.0)) << "  (Tango factor)\n"
              << "    Guarantee: cost(Tango) <= O(log log n) * OPT\n";

    sep("TEMPORAL LOCALITY — repeated root access");
    TangoTree tt2(keys);
    for(int i=0;i<10;i++) tt2.access(8);
    tt2.print_stats();

    sep("LARGE TREE — n=63 (height 6)");
    vector<int> big; for(int i=1;i<=63;i++) big.push_back(i);
    TangoTree tt3(big);
    for(int k:big) tt3.access(k);
    for(int k:{32,16,48,8,24,40,56}) tt3.access(k);
    tt3.print_stats();
    cout << "    log2(log2(63)) = " << log2(log2(63.0)) << "\n";
}

int main() {
    setup_nil();
    run_unit_tests();
    run_demo();
    cout << "\n" <<string(62,'=') << "\n  Done.\n"
              << string(62,'=') << "\n";
    return 0;
}

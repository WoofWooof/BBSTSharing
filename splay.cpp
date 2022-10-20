#include <cstdio>
#include <iostream>
using namespace std;
const int SIZE = 2e6 + 6;
const int INF = 2147483647;

namespace Splay {
struct Infomat {
	int value, num;
} cont[SIZE];
int parent[SIZE], child[SIZE][2];
int siz[SIZE];
int root, cnt;

void updat(int v) {
	siz[v] = cont[v].num + siz[child[v][0]] + siz[child[v][1]];
}

void rotat(int v) {
	int p = parent[v], is_rightchild = v == child[p][1];
	if (parent[p])
		child[parent[p]][p == child[parent[p]][1]] = v;
	parent[v] = parent[p];
	child[p][is_rightchild] = child[v][is_rightchild ^ 1];
	if (child[p][is_rightchild])
		parent[child[p][is_rightchild]] = p;
	child[v][is_rightchild ^ 1] = p;
	parent[p] = v;
	updat(p);
	updat(v);
}

void splay(int v, int goal, int& root) {
	int p, g;
	updat(v);
	while (parent[v] != goal) {
		p = parent[v], g = parent[p];
		if (g != goal && (v == child[p][1]) == (p == child[g][1]))
			rotat(p);
		rotat(v);
	}
	if (!goal)
		root = v;
}

int search_by_value(int vl, int& root) {
	int v = root;
	while (1) {
		if (cont[v].value == vl)
			break;
		if (cont[v].value < vl) {
			if (child[v][1])
				v = child[v][1];
			else
				break;
		} else {
			if (child[v][0])
				v = child[v][0];
			else
				break;
		}
	}
	return v;
}

int search_by_rank(int rk, int& root) {
	int v = root;
	while (1) {
		if (siz[child[v][0]] >= rk)
			v = child[v][0];
		else if (siz[child[v][0]] + cont[v].num < rk)
			rk -= siz[child[v][0]] + cont[v].num, v = child[v][1];
		else
			break;
	}
	return v;
}

int get_prev_element(int v) {
	v = child[v][0];
	while (child[v][1])
		v = child[v][1];
	return v;
}

int get_succ_element(int v) {
	v = child[v][1];
	while (child[v][0])
		v = child[v][0];
	return v;
}

void insert_element(int vl, int& root) {
	if (!root) {
		int u = ++cnt;
		cont[u].value = vl, cont[u].num = 1;
		splay(u, 0, root);
		return;
	}
	int v = search_by_value(vl, root);
	if (cont[v].value == vl) {
		++cont[v].num;
		splay(v, 0, root);
		return;
	}
	int u = ++cnt;
	cont[u].value = vl, cont[u].num = 1;
	if (cont[v].value < vl) {
		parent[u] = v;
		child[v][1] = u;
		splay(u, 0, root);
	} else {
		parent[u] = v;
		child[v][0] = u;
		splay(u, 0, root);
	}
}

void delet_element(int vl, int& root) { // erase
	int v = search_by_value(vl, root);
	--cont[v].num;
	splay(v, 0, root);
	if (!cont[v].num) {
		if (child[v][0]) {
			int u = get_prev_element(v);
			splay(u, v, root);
			child[u][1] = child[v][1];
			if (child[u][1])
				parent[child[u][1]] = u;
			parent[u] = 0;
			splay(u, 0, root);
		} else {
			int u = child[v][1];
			if (u) {
				parent[u] = 0;
				splay(u, 0, root);
			} else {
				root = 0;
			}
		}
	}
}

int get_rank(int vl, int& root) {
	int v = search_by_value(vl, root);
	splay(v, 0, root);
	if (cont[v].value >= vl)
		return siz[child[v][0]] + 1;
	if (!child[v][1])
		return siz[root] + 1;
	int u = get_succ_element(v);
	splay(u, 0, root);
	return siz[child[u][0]] + 1;
}

int get_value(int rk, int& root) {
	int v = search_by_rank(rk, root);
	splay(v, 0, root);
	return cont[v].value;
}

int a_redundant_operat(int vl, int& root) {
	if (!root)
		return -INF;
	int v = search_by_value(vl, root);
	splay(v, 0, root);
	if (cont[v].value < vl)
		return cont[v].value;
	if (!child[v][0])
		return -INF;
	int u = get_prev_element(v);
	splay(u, 0, root);
	return cont[u].value;
}

int lower_bound(int vl, int& root) {
	if (!root)
		return INF;
	int v = search_by_value(vl, root);
	splay(v, 0, root);
	return cont[v].value;
}

int upper_bound(int vl, int& root) {
	if (!root)
		return INF;
	int v = search_by_value(vl, root);
	splay(v, 0, root);
	if (cont[v].value > vl)
		return cont[v].value;
	if (!child[v][1])
		return INF;
	int u = get_succ_element(v);
	splay(u, 0, root);
	return cont[u].value;
}
} // namespace Splay

using namespace Splay;

namespace Splay_for_array {
int root, cnt, parent[N], child[N][2], reverse_flag[N], siz[N];

void refresh_flag(int v) {
	if (reverse_flag[v]) {
		if (child[v][0] != -1)
			reverse_flag[child[v][0]] ^= 1;
		if (child[v][1] != -1)
			reverse_flag[child[v][1]] ^= 1;
		swap(child[v][0], child[v][1]);
		reverse_flag[v] = 0;
	}
}

void updat(int v) { siz[v] = siz[child[v][0]] + siz[child[v][1]] + 1; }

void rotat(int v) {
	refresh_flag(parent[v]);
	refresh_flag(v);
	int p = parent[v], w = (v == child[p][1]);
	if (parent[p] != -1)
		child[parent[p]][p == child[parent[p]][1]] = v;
	parent[v] = parent[p];
	child[p][w] = child[v][w ^ 1];
	if (child[p][w] != -1)
		parent[child[p][w]] = p;
	parent[p] = v;
	child[v][w ^ 1] = p;
	updat(p);
	updat(v);
}

void splay(int v, int gl) {
	int p, g;
	updat(v);
	while (parent[v] != gl) {
		p = parent[v];
		g = parent[p];
		refresh_flag(g);
		refresh_flag(p);
		if (g != gl && (v == child[p][1]) == (p == child[g][1]))
			rotat(p);
		rotat(v);
	}
	if (gl == -1)
		root = v;
}

int build(int l, int r, int p) {
	if (l > r)
		return -1;
	int mid = (l + r) >> 1;
	siz[mid] = r - l + 1;
	parent[mid] = p;
	child[mid][0] = build(l, mid - 1, mid);
	child[mid][1] = build(mid + 1, r, mid);
	return mid;
}

void init(int l, int r) {
	cnt = 0;
	root = build(l, r, -1);
}

int search_by_rank(int x) {
	int v = root;
	refresh_flag(v);
	while (x != siz[child[v][0]] + 1) {
		if (siz[child[v][0]] >= x)
			v = child[v][0];
		else {
			x -= siz[child[v][0]] + 1;
			v = child[v][1];
		}
		refresh_flag(v);
	}
	return v;
}

void reverse_interv(int x, int y) {
	x = search_by_rank(x);
	y = search_by_rank(y + 2);
	splay(x, -1);
	splay(y, x);
	reverse_flag[child[y][0]] ^= 1;
}

void trav_interv(int v, int x, int y) {
	refresh_flag(v);
	if (child[v][0] != -1)
		trav_interv(child[v][0], x, y);
	if (x <= v && v <= y)
		printf("%d ", v);
	if (child[v][1] != -1)
		trav_interv(child[v][1], x, y);
}

void print_for_debug(int l, int r) {
	trav_interv(root, l, r);
	printf("\n");
}
} // namespace Splay_for_array

int main() {
	int n, m, last = 0, ans = 0;
	scanf("%d%d", &n, &m);
	for (int i = 0; i < n; i++) {
		int x;
		scanf("%d", &x);
		insert_element(x, root);
	}
	for (int i = 0; i < m; i++) {
		int opt, x;
		scanf("%d%d", &opt, &x);
		x ^= last;
		if (opt == 1) {
			insert_element(x, root);
		}
		if (opt == 2) {
			delet_element(x, root);
		}
		if (opt == 3) {
			ans ^= last = get_rank(x, root);
		}
		if (opt == 4) {
			ans ^= last = get_value(x, root);
		}
		if (opt == 5) {
			ans ^= last = a_redundant_operat(x, root);
		}
		if (opt == 6) {
			ans ^= last = upper_bound(x, root);
		}
	}
	printf("%d\n", ans);
	return 0;
}
#include <cstdio>
#include <iostream>
using namespace std;

class Node {
public:
	int data, hight;
	int num, siz;
	Node* left_child;
	Node* right_child;
	Node(int a = 0)
	    : data(a)
	    , hight(1)
	    , num(1)
	    , siz(1)
	    , left_child(NULL)
	    , right_child(NULL) {}
	~Node() {}
};

Node* root;

int get_siz(Node* p) {
	if (p == NULL)
		return 0;
	return p->siz;
}

int get_hight(Node* p) {
	if (p == NULL)
		return 0;
	return p->hight;
}

void updat(Node*& p) {
	p->siz = get_siz(p->left_child) + get_siz(p->right_child) + p->num;
	p->hight = max(get_hight(p->left_child), get_hight(p->right_child)) + 1;
}
//左旋，这里我忘了是zig还是zag了
void zig(Node*& p) {
	Node* q;
	q = p->left_child;
	p->left_child = q->right_child;
	q->right_child = p;
	updat(p);
	updat(q);
	p = q;
}

void zag(Node*& p) {
	Node* q;
	q = p->right_child;
	p->right_child = q->left_child;
	q->left_child = p;
	updat(p);
	updat(q);
	p = q;
}

void zigzag(Node*& p) {
	zag(p->left_child);
	zig(p);
}

void zagzig(Node*& p) {
	zig(p->right_child);
	zag(p);
}

void rebuild_34(Node*& p) {
	// 3,4-重构
	//这种实现非常繁琐，希望有人能给他简化一下
	Node *a, *b, *c;
	Node *A, *B, *C, *D;
	if (get_hight(p->left_child) > get_hight(p->right_child)) {
		c = p;
		D = c->right_child;
		if (get_hight(p->left_child->left_child) >
		    get_hight(p->left_child->right_child))
			b = p->left_child, a = b->left_child, A = a->left_child,
			B = a->right_child, C = b->right_child;
		else
			a = p->left_child, b = a->right_child, A = a->left_child,
			B = b->left_child, C = b->right_child;
	} else {
		a = p;
		A = a->left_child;
		if (get_hight(p->right_child->left_child) >
		    get_hight(p->right_child->right_child))
			c = p->right_child, b = c->left_child, B = b->left_child,
			C = b->right_child, D = c->right_child;
		else
			b = p->right_child, c = b->right_child, B = b->left_child,
			C = c->left_child, D = c->right_child;
	}
	p = b;
	b->left_child = a;
	b->right_child = c;
	a->left_child = A;
	a->right_child = B;
	c->left_child = C;
	c->right_child = D;
	updat(a);
	updat(c);
	updat(b);
}

void insert_element(Node*& p, int x) {
	if (p == NULL) {
		p = new Node(x);
		return;
	}
	if (p->data == x) {
		++(p->num);
		updat(p);
		return;
	}
	if (p->data > x) {
		insert_element(p->left_child, x), updat(p);
		if (get_hight(p->left_child) - get_hight(p->right_child) == 2) {
			/*可以替换下面单个语句
			if (x < p->left_child->data)
			    zig(p);
			else
			    zigzag(p);
			*/
			rebuild_34(p);
		}
	} else {
		insert_element(p->right_child, x), updat(p);
		if (get_hight(p->right_child) - get_hight(p->left_child) == 2) {
			/*
			if (x > p->right_child->data)
			    zag(p);
			else
			    zagzig(p);
			*/
			rebuild_34(p);
		}
	}
	updat(p);
}

void delet_element(Node*& p, int x) {
	if (p == NULL)
		return;
	if (p->data > x) {
		delet_element(p->left_child, x), updat(p);
		if (get_hight(p->right_child) - get_hight(p->left_child) == 2) {
			if (get_hight(p->right_child->right_child) >=
			    get_hight(p->right_child->left_child))
				zag(p);
			else
				zagzig(p);
		}
	} else if (p->data < x) {
		delet_element(p->right_child, x), updat(p);
		if (get_hight(p->left_child) - get_hight(p->right_child) == 2) {
			if (get_hight(p->left_child->left_child) >=
			    get_hight(p->left_child->right_child))
				zig(p);
			else
				zigzag(p);
		}
	} else {
		if (p->num > 1) {
			--(p->num);
			updat(p);
			return;
		}
		if (p->left_child && p->right_child) {
			Node* q = p->right_child;
			while (q->left_child)
				q = q->left_child;
			p->num = q->num;
			p->data = q->data, q->num = 0;
			delet_element(p->right_child, q->data);
			updat(p);
			if (get_hight(p->left_child) - get_hight(p->right_child) == 2) {
				if (get_hight(p->left_child->left_child) >=
				    get_hight(p->left_child->right_child))
					zig(p);
				else
					zigzag(p);
			}
		} else {
			Node* q = p;
			if (p->left_child)
				p = p->left_child;
			else if (p->right_child)
				p = p->right_child;
			else
				p = NULL;
			delete q;
			q = NULL;
		}
	}
	if (p)
		updat(p);
}

int search_by_value(Node* p, int val) {
	if (p == NULL)
		return 1;
	if (p->data == val)
		return get_siz(p->left_child) + 1;
	if (p->data > val)
		return search_by_value(p->left_child, val);
	return search_by_value(p->right_child, val) + get_siz(p->left_child) +
	       p->num;
}

int search_by_rank(Node* p, int rank) {
	if (get_siz(p->left_child) >= rank)
		return search_by_rank(p->left_child, rank);
	if (get_siz(p->left_child) + p->num >= rank)
		return p->data;
	return search_by_rank(p->right_child,
	                      rank - get_siz(p->left_child) - p->num);
}

int get_lower(int val) {
	Node* p = root;
	int ans = -2147483648;
	while (p) {
		if (p->data == val) {
			if (p->left_child) {
				p = p->left_child;
				while (p->right_child)
					p = p->right_child;
				ans = p->data;
			}
			break;
		}
		if (p->data < val && p->data > ans)
			ans = p->data;
		p = p->data < val ? p->right_child : p->left_child;
	}
	return ans;
}

int get_upper(int val) {
	Node* p = root;
	int ans = 2147483647;
	while (p) {
		if (p->data == val) {
			if (p->right_child) {
				p = p->right_child;
				while (p->left_child)
					p = p->left_child;
				ans = p->data;
			}
			break;
		}
		if (p->data > val && p->data < ans)
			ans = p->data;
		p = p->data < val ? p->right_child : p->left_child;
	}
	return ans;
}

void clear(Node*& p) {
	if (p->left_child)
		clear(p->left_child);
	if (p->right_child)
		clear(p->right_child);
	delete (p);
}

int main() {
	int n;
	scanf("%d", &n);
	for (int i = 0; i < n; i++) {
		int opt, x;
		scanf("%d%d", &opt, &x);
		switch (opt) {
		case 1:
			insert_element(root, x);
			break;
		case 2:
			delet_element(root, x);
			break;
		case 3:
			printf("%d\n", search_by_value(root, x));
			break;
		case 4:
			printf("%d\n", search_by_rank(root, x));
			break;
		case 5:
			printf("%d\n", get_lower(x));
			break;
		case 6:
			printf("%d\n", get_upper(x));
			break;
		}
	}
	clear(root);
	return 0;
}
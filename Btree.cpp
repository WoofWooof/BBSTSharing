#include <cstdio>
#include <iostream>
using namespace std;
template <typename TYP, int BLOK_SIZ> class BTree {
public:
	struct Infomat {
		TYP value;
		int num;
	};

private:
	struct Node {
		Infomat cont[2 * BLOK_SIZ - 1];
		Node* child[2 * BLOK_SIZ] = {nullptr};
		Node* parent = nullptr;
		int keyNum = 0, size = 0;
		bool isLeaf = true;
		const TYP& key(int i) const { return cont[i].value; }
		int& cnt(int i) { return cont[i].num; }
		Node(Node* parent = nullptr)
		    : parent(parent) {}
	};
	Node* root = nullptr;

	static bool pairComp(const Infomat& lhs, const TYP& rhs) {
		return lhs.value < rhs;
	}

	template <typename T> static void shiftBy(T* ptr, int length, int shift) {
		memmove(ptr + shift, ptr, length * sizeof(T));
	}

	static int calcSize(Node* x) {
		if (!x)
			return 0;
		int nsz = 0;
		for (int i = 0; i < x->keyNum; ++i)
			nsz += getSize(x->child[i]) + x->cnt(i);
		nsz += getSize(x->child[x->keyNum]);
		return nsz;
	}

	static int getSize(Node* x) {
		if (!x)
			return 0;
		return x->size;
	}
	//把where孩子分成两个节点，都作为x的孩子
	void split(Node* x, int where) {
		Node* z = new Node(x);
		Node* y = x->child[where];
		z->isLeaf = y->isLeaf;
		memmove(z->cont, y->cont + BLOK_SIZ, (BLOK_SIZ - 1) * sizeof(Infomat));
		if (!y->isLeaf) {
			memmove(z->child, y->child + BLOK_SIZ, BLOK_SIZ * sizeof(Node*));
			for (int i = 0; i < BLOK_SIZ; ++i)
				z->child[i]->parent = z;
		}
		z->keyNum = y->keyNum = BLOK_SIZ - 1;
		shiftBy(x->child + where + 1, x->keyNum - where,
		        1); //注意child本身keyNum多一个
		x->child[where + 1] = z;
		shiftBy(x->cont + where, x->keyNum - where, 1);
		new (x->cont + where) Infomat(y->cont[BLOK_SIZ - 1]);

		y->size = calcSize(y), z->size = calcSize(z);
		++x->keyNum;
	}

	void insertEmpty(Node* x, const TYP& key) {
		while (true) {
			int i =
			    lower_bound(x->cont, x->cont + x->keyNum, key, pairComp) -
			    x->cont;
			if (i != x->keyNum && !(key < x->cont[i].value)) //重复插入
			{
				++x->cnt(i);
				while (x)
					++x->size, x = x->parent;
				return;
			}
			if (x->isLeaf) {
				shiftBy(x->cont + i, x->keyNum - i, 1);
				x->cont[i] = {key, 1};
				++x->keyNum;
				while (x)
					++x->size, x = x->parent;
				return;
			}
			if (x->child[i]->keyNum == 2 * BLOK_SIZ - 1) {
				split(x, i);
				if (x->key(i) < key)
					++i;
				else if (!(key < x->key(i))) {
					++x->cnt(i);
					while (x)
						++x->size, x = x->parent;
					return;
				}
			}
			x = x->child[i];
		}
	}

	void merge(Node* x, int i)
	//将x的i孩子与i+1孩子合并，用x的i键作为分隔，这两个孩子都只有BF-1个孩子，合并后有2*BLOK_SIZ-1个
	{
		Node *y = x->child[i], *z = x->child[i + 1];
		y->keyNum = 2 * BLOK_SIZ - 1;
		y->cont[BLOK_SIZ - 1] = std::move(x->cont[i]);
		memmove(y->cont + BLOK_SIZ, z->cont, (BLOK_SIZ - 1) * sizeof(Infomat));
		if (!y->isLeaf) {
			memmove(y->child + BLOK_SIZ, z->child, BLOK_SIZ * sizeof(Node*));
			for (int j = BLOK_SIZ; j <= 2 * BLOK_SIZ - 1; ++j)
				y->child[j]->parent = y;
		}
		shiftBy(x->cont + i + 1, x->keyNum - i - 1, -1);
		shiftBy(x->child + i + 2, x->keyNum - i - 1, -1);

		--x->keyNum;
		y->size = calcSize(y);
	}
    
	void erase(Node* x, const TYP& key) {
		int i = lower_bound(x->cont, x->cont + x->keyNum, key, pairComp) -
		        x->cont;
		if (i != x->keyNum && !(key < x->cont[i].value)) //找到key了
		{
			if (x->cnt(i) > 1) {
				--x->cnt(i);
				while (x)
					--x->size, x = x->parent;
				return;
			}
			if (x->isLeaf) // x是叶节点，直接删除
			{
				shiftBy(x->cont + i + 1, --x->keyNum - i, -1);
				//需要移动的内存是x->keyNum-i-1
				while (x)
					--x->size, x = x->parent;
			} else {
				if (x->child[i]->keyNum >= BLOK_SIZ)
				//前驱所在孩子有足够的孩子(以应对它的孩子的需求)
				{
					Node* y = x->child[i];
					while (!y->isLeaf)
						y = y->child[y->keyNum]; //找前驱
					x->cont[i] = y->cont[y->keyNum - 1];
					if (x->cnt(i) != 1)
					// y的对应节点cnt有多个，那么沿路减size;只有一个的话删除的时候会处理
					{
						y->cnt(y->keyNum - 1) = 1;
						while (y != x)
							y->size -= x->cnt(i) - 1, y = y->parent;
					}

					erase(x->child[i], x->key(i));
				} else if (x->child[i + 1]->keyNum >= BLOK_SIZ)
				//后继所在孩子有足够的孩子
				{
					Node* y = x->child[i + 1];
					while (!y->isLeaf)
						y = y->child[0]; //找后继
					x->cont[i] = y->cont[0];
					if (x->cnt(i) != 1) {
						y->cnt(0) = 1;
						while (y != x)
							y->size -= x->cnt(i) - 1, y = y->parent;
					}

					erase(x->child[i + 1], x->key(i));
				} else //都没有,那么把这两个节点都合并到y中，并且挪动x的孩子和键
				{
					merge(x, i);
					if (root->keyNum == 0)
						// keyNum==0只是没有键了，但是还可能有一个孩子，这时根变成这个孩子
						root = x->child[i], root->parent = nullptr;
					erase(x->child[i], key);
				}
			}
		} else if (!x->isLeaf)
		//没有找到key,只要保证x->child[i]->keyNum足够多即可无脑递归，然而很难保证
		{
			if (x->child[i]->keyNum == BLOK_SIZ - 1) {
				Node* y = x->child[i];
				if (i >= 1 && x->child[i - 1]->keyNum >= BLOK_SIZ)
				//左兄弟，取走它的最大孩子
				{
					//找相邻的兄弟借节点，类似旋转操作,把x的一个键移入要删的key所在孩子，把它的兄弟的一个key和孩子移入x
					//但是从左还是右借并不完全一样，所以不能一概处理
					Node* z = x->child[i - 1];
					shiftBy(y->cont, y->keyNum, 1);
					//是否需要考虑析构的问题？z的keyNum已经减了，
					//不可能再去析构z->cont[z->keyNum - 1]了
					//所以，value的构造必须要用new不能用=，从而避开=的资源释放
					//但是value的移动似乎应该是bitwise的，考虑std::move
					new (y->cont) Infomat(std::move(x->cont[i - 1]));
					new (x->cont + i - 1)
					    Infomat(std::move(z->cont[z->keyNum - 1]));
					if (!y->isLeaf) {
						shiftBy(y->child, y->keyNum + 1, 1);
						y->child[0] = z->child[z->keyNum], y->child[0]->parent = y;
					}

					--z->keyNum, ++y->keyNum;
					y->size = calcSize(y), z->size = calcSize(z);
					erase(y, key);
				} else if (i < x->keyNum && x->child[i + 1]->keyNum >= BLOK_SIZ)
				//右兄弟,取走它的最小孩子
				{
					Node* z = x->child[i + 1];
					new (y->cont + y->keyNum)
					    Infomat(std::move(x->cont[i]));
					new (x->cont + i) Infomat(std::move(z->cont[0]));
					if (!y->isLeaf) // y和z深度一样，isLeaf情况相同
					{
						y->child[y->keyNum + 1] = z->child[0],
						                     y->child[y->keyNum + 1]->parent = y;
						shiftBy(z->child + 1, z->keyNum, -1);
					}
					shiftBy(z->cont + 1, z->keyNum - 1, -1);

					--z->keyNum, ++y->keyNum;
					y->size = calcSize(y), z->size = calcSize(z);
					erase(y, key);
				} else //两个兄弟都没有节点借,那么将它与随便左右哪个兄弟合并，然而还是要特判一下
				{
					if (i != 0)
						--i; // i==0时，y与y+1合并仍放于y；否则y与y-1合并放于y-1
					y = x->child[i];
					merge(x, i);
					if (root->keyNum == 0)
						root = y, root->parent = nullptr;
					erase(y, key);
				}
			} else
				erase(x->child[i], key);
		}
	}

public:
	BTree()
	    : root(new Node) {}
	void insert(const TYP& key) {
		//沿路向下分裂满节点,每次分裂成左右一半，孩子的中间key留在父亲节点中用于分隔两个新孩子
		// insertEmpty只保证了当前节点有空间(来容纳它的孩子的分裂)，不保证key需要去的孩子节点也有空间
		if (root->keyNum == 2 * BLOK_SIZ - 1) {
			Node* x = new Node;
			x->isLeaf = false, x->child[0] = root,
			x->size = root->size; //+1操作由insertEmpty来做
			root->parent = x, root = x;
			split(x, 0); // split接受参数：node的满子节点下标
		}
		insertEmpty(root, key);
	}
	void erase(const TYP& key) { erase(root, key); }
	int next(const TYP& key) {
		Node* x = root;
		int ret;
		while (x) {
			int i =
			    lower_bound(x->cont, x->cont + x->keyNum, key, pairComp) -
			    x->cont;
			if (x->cont[i].value == key)
				++i;
			if (i != x->keyNum)
				ret = x->cont[i].value;
			x = x->child[i];
		}
		return ret;
	}
	int prev(const TYP& key) {
		Node* x = root;
		int ret;
		while (x) {
			int i =
			    lower_bound(x->cont, x->cont + x->keyNum, key, pairComp) -
			    x->cont;
			if (i)
				ret = x->cont[i - 1].value;
			x = x->child[i];
		}
		return ret;
	}
	int rank(const TYP& key) {
		Node* x = root;
		int ret = 0;
		while (x) {
			if (x->key(x->keyNum - 1) < key) {
				ret += x->size - getSize(x->child[x->keyNum]);
				x = x->child[x->keyNum];
				continue;
			}
			for (int i = 0; i < x->keyNum; ++i) {
				if (x->key(i) < key)
					ret += getSize(x->child[i]) + x->cnt(i);
				else if (x->key(i) == key)
					return ret + getSize(x->child[i]) + 1;
				else {
					x = x->child[i];
					break;
				}
			}
		}
		return ret;
	}
	int kth(int k) {
		Node* x = root;
		while (true) {
			for (int i = 0; i <= x->keyNum; ++i) {
				// const int csz = getSize(x->child[i]) + (i == x->keyNum ? 1 :
				// x->cnt(i));
				const int lb = getSize(x->child[i]) + 1,
				          ub = getSize(x->child[i]) +
				               (i == x->keyNum ? 1 : x->cnt(i));
				if (k >= lb && k <= ub)
					return x->key(i);
				if (k < lb) {
					x = x->child[i];
					break;
				}
				k -= ub;
			}
		}
	}
};

int main() {
	BTree<int, 35> bt;
	int n, opt, x;
	scanf("%d", &n);
	while (n--) {
		scanf("%d", &opt, &x);
		switch (opt) {
		case 1:
			bt.insert(x);
			break;
		case 2:
			bt.erase(x);
			break;
		case 3:
			printf("%d\n", bt.rank(x));
			break;
		case 4:
			printf("%d\n", bt.kth(x));
			break;
		case 5:
			printf("%d\n", bt.prev(x));
			break;
		case 6:
			printf("%d\n", bt.next(x));
			break;
		}
	}
	return 0;
}

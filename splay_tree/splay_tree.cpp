#include <iostream>

template <typename T>
class SplayTree {
private:
    // tree node structure
    struct Node {
        Node *leftChild;  // pointer to left child
        Node *rightChild; // pointer to right child
        Node *parent;     // pointer to parent
        T data;           // data field

        // constructor
        Node(const T &data)
        : leftChild(nullptr), rightChild(nullptr), parent(nullptr), data(data) {}

        // destructor
        ~Node() = default;
    } *root;

    // updates pointers to child (for grandpa) and parent (for child)
    // seems like swap, but after that necessary link old child and old parent
    void updateParents(Node *parent, Node *child) {
        // update child's grandpa's pointer
        if (parent->parent == nullptr) {
            root = child;
        } else if (parent == parent->parent->leftChild) {
            parent->parent->leftChild = child;
        } else if (parent == parent->parent->rightChild) {
            parent->parent->rightChild = child;
        }

        // update child pointer
        if (child) {
            child->parent = parent->parent;
        }
    }

    // performs left rotation (rotation of the edge connecting n and it's right child)
    void leftRotate(Node *n) {
        Node* rightChild = n->rightChild;

        // execute shift left child of the n's right child to n's right child
        // (even if equal nullptr)
        n->rightChild = rightChild->leftChild;
        if (rightChild->leftChild) {
            rightChild->leftChild->parent = n;
        }

        updateParents(n, rightChild);

        // link old child and old parent
        rightChild->leftChild = n;
        rightChild->leftChild->parent = rightChild;
    }

    // performs right rotation (rotation of the edge connecting n and it's left child)
    void rightRotate(Node *n) {
        Node *leftChild = n->leftChild;

        // execute shift right child of the n's left child to n's left child
        // (even if equal nullptr)
        n->leftChild = leftChild->rightChild;
        if (leftChild->rightChild) {
            leftChild->rightChild->parent = n;
        }

        updateParents(n, leftChild);

        // link old child and old parent
        leftChild->rightChild = n;
        leftChild->rightChild->parent = leftChild;
    }

    // performs splay (delivers a node n to the root of the tree using left and right pivots)
    void splay(Node *n) {
        while (parent(n)) {
            // case: node n is left child
            if (n == parent(n)->leftChild) {
                if (!grandparent(n)) { // there isn't grandparent
                    // zig operation
                    rightRotate(parent(n));
                } else if (grandparent(n)->leftChild == parent(n)) { // n's parent - left child
                    // zig-zig operation
                    rightRotate(grandparent(n));
                    rightRotate(parent(n));
                } else { // n's parent - right child
                    // zig-zag operation
                    rightRotate(parent(n));
                    leftRotate(parent(n));
                }
            } else { // case: node n is right child
                if (!grandparent(n)) { // there isn't grandparent
                    // zig operation
                    leftRotate(parent(n));
                } else if (grandparent(n)->rightChild == parent(n)) { // n's parent - right child
                    // zig-zig operation
                    leftRotate(grandparent(n));
                    leftRotate(parent(n));
                } else { // n's parent - left child
                    // zig-zag operation
                    leftRotate(parent(n));
                    rightRotate(parent(n));
                }
            }
        }
    }

    // returns parent of the n node
    Node* parent(const Node *n) const {
        return n->parent;
    }

    // returns grandparent of the n node
    Node* grandparent(const Node *n) const {
        return n->parent->parent;
    }

    // returns minimum from subtree with root n
    Node* _minimum(Node *n) const {
        Node *minimum = n;
        while (minimum->leftChild) {
            minimum = minimum->leftChild;
        }
        return minimum;
    }

    // returns maximum from subtree with root n
    Node* _maximum(Node *n) const {
        Node *maximum = n;
        while (maximum->rightChild) {
            maximum = maximum->rightChild;
        }
        return maximum;
    }
public:
    SplayTree() : root(nullptr) {}

    // inserts an element with data insertingData
    void insert(const T &insertingData) {
        Node *insertPlace = root;
        Node *parentOfInsertedPlace = nullptr;

        // finding suitable place
        while (insertPlace) {
            // necessary save previous insertPlace (it will be a parent of the insertPlace)
            parentOfInsertedPlace = insertPlace;
            if (insertingData > insertPlace->data) {
                insertPlace = insertPlace->rightChild;
            } else {
                insertPlace = insertPlace->leftChild;
            }
        }

        // creating inserting element
        Node *insertedNode = new Node(insertingData);
        insertedNode->parent = parentOfInsertedPlace;

        // link inserted element's parent with him
        if (!parentOfInsertedPlace) {
            root = insertedNode;
        } else if (parentOfInsertedPlace->data < insertedNode->data) {
            parentOfInsertedPlace->rightChild = insertedNode;
        } else {
            parentOfInsertedPlace->leftChild = insertedNode;
        }

        splay(insertedNode);
    }

    // returns a pointer to the node with data searchingData
    Node* find(const T &searchingData) {
        Node* findingNode = root;

        while (findingNode) {
            if (searchingData > findingNode->data) {
                findingNode = findingNode->rightChild;
            } else if (searchingData < findingNode->data) {
                findingNode = findingNode->leftChild;
            } else {
                splay(findingNode);
                return findingNode;
            }
        }

        return nullptr;
    }

    // removes an element with data deletingData
    void erase(const T &deletingData) {
        Node* deletingNode = find(deletingData);

        if (deletingNode) {
            if (deletingNode->rightChild == nullptr) {
                updateParents(deletingNode, deletingNode->leftChild);
            } else if (deletingNode->leftChild == nullptr) {
                updateParents(deletingNode, deletingNode->rightChild);
            } else { // case: there are two children owned deletingNode
                Node* replacementRemotedNode = _maximum(deletingNode->leftChild);

                // case: replacing node can have subtrees
                if (parent(replacementRemotedNode) != deletingNode) {
                    updateParents(replacementRemotedNode, replacementRemotedNode->leftChild);

                    replacementRemotedNode->leftChild = deletingNode->leftChild;
                    replacementRemotedNode->leftChild->parent = replacementRemotedNode;
                }

                updateParents(deletingNode, replacementRemotedNode);

                replacementRemotedNode->rightChild = deletingNode->rightChild;
                deletingNode->rightChild->parent = replacementRemotedNode;
            }

            if (parent(deletingNode)) {
                splay(parent(deletingNode));
            }
            delete deletingNode;
        }
    }

    // merges this tree with tree addedTree
    void merge(const SplayTree &addedTree) {
        // necessary make sure that all keys of the current (this) tree less than the keys of the tree T2
        // and remember "smallest" tree
        SplayTree smallest, largest;
        if (this->root->data > addedTree.root->data) {
            smallest.root = addedTree.root;
            largest.root = this->root;
        } else {
            smallest.root = this->root;
            largest.root = addedTree.root;
        }

        smallest.splay(_maximum(smallest.root));

        smallest.root->rightChild = largest.root;
        this->root = smallest.root;
    }

    // splits the tree into two trees
    // first tree going to be in "this" tree, second in secondTree
    // first tree contains elements less or equal than splitElement
    // second tree contains elements greater than x
    // note: not necessary splitElement there's in the tree
    void split(SplayTree &secondTree, const T &splitElement) {
        Node* similarOrEqualToSplitElement = root;

        // finding element, which similar to splitElement (even can be equal)
        while (similarOrEqualToSplitElement) {
            if (splitElement < similarOrEqualToSplitElement->data && similarOrEqualToSplitElement->leftChild) {
                similarOrEqualToSplitElement = similarOrEqualToSplitElement->leftChild;
            } else if (splitElement > similarOrEqualToSplitElement->data && similarOrEqualToSplitElement->rightChild) {
                similarOrEqualToSplitElement = similarOrEqualToSplitElement->rightChild;
            } else {
                break;
            }
        }

        splay(similarOrEqualToSplitElement);

        secondTree.root = similarOrEqualToSplitElement->rightChild;
        similarOrEqualToSplitElement->rightChild = nullptr;
    }

    // returns minimum of the tree
    T minimum() const {
        return _minimum(root)->data;
    }

    // returns maximum of the tree
    T maximum() const {
        return _maximum(root)->data;
    }

    // returns "true", if tree is empty, otherwise returns "false"
    bool empty() const {
        return (root == nullptr);
    }
};
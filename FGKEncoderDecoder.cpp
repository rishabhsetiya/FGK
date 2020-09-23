#include<iostream>
#include<fstream>
#include<vector>
#include<queue>
#include<unordered_map>
#include<algorithm>
using namespace std;

bool nextbit(string filename){
    static int bitsremaining=0, currentposition=0;
    static char cur;
    if(bitsremaining<=0){
        ifstream mf(filename, ios::binary);
        mf.seekg(currentposition, mf.beg);
        bitsremaining=8;
        mf.read((char*)&cur, sizeof(char));
        currentposition++;
    }
    bool ans=(cur>>7)&1;
    cur<<=1;
    bitsremaining--;
    return ans;
}

void writebit(bool x, bool last, string filename){
    static int curno;
    static char cur;

    if(last){
        if(curno==0){
            ofstream nf(filename, ios::binary | ios::app);
            cur=0;
            nf.write((char*)&cur, sizeof(char));
            return;
        }
        cur<<=(8-curno);
        ofstream nf(filename, ios::binary | ios::app);
        nf.write((char*)&cur, sizeof(char));
        cur=8-curno;
        nf.write((char*)&cur, sizeof(char));
        return;
    }
    curno++;
    cur<<=1;
    cur+=x;
    if(curno>=8){
        ofstream nf(filename, ios::binary | ios::app);
        nf.write((char*)&cur, sizeof(char));
        curno=0;
    }
}

class Huffman{

    struct Node{
        char c;
        int weight;
        Node *parent;
        Node *left;
        Node *right;
    };

    Node *root;
    unordered_map<char, Node*> getNode;
    string inputfile, outputfile;

    vector<Node*> levelOrder() {
        vector<Node*> ans;
        if(!root) return ans;
        queue<Node*> q;
        q.push(root);
        while(!q.empty()){
            int levelsize=q.size();
            for(int i=0; i<levelsize; i++){
                if(q.front()->right)
                    q.push(q.front()->right);
                if(q.front()->left)
                    q.push(q.front()->left);
            ans.push_back(q.front());
            q.pop();
            }
        }
        reverse(ans.begin(), ans.end());
        return ans;
    }

    void update(char a){
        if(getNode.find(a)==getNode.end()){
            Node *p=new Node();
            p->c=a;
            p->weight=0;
            p->parent=getNode['\0'];
            getNode[a]=p;
            getNode['\0']->right=p;

            //Creating new NYT node
            Node *n=new Node();
            n->c='\0';
            n->weight=0;
            n->parent=getNode['\0'];
            n->left=nullptr;
            n->right=nullptr;

            getNode['\0']->left=n;
            getNode['\0']=n;
        }

        Node *temp=getNode[a];
        while(temp!=nullptr){
                temp->weight++;
                temp=temp->parent;
        }


        while(true)
        {
        vector<Node*> levelorder=levelOrder();
        int i;
        for(i=1; i<levelorder.size()-1; i++)
            if(levelorder[i]->weight>levelorder[i+1]->weight)
                break;
        if(i==levelorder.size()-1)
            return;
        int j;
        for(j=levelorder.size()-1; j>=0; j--)
            if(levelorder[j]->weight==levelorder[i]->weight-1)
                break;
        if(j<i)
            return;
        //Swap nodes at position i and j
        Node *parentofi=levelorder[i]->parent;
        Node *parentofj=levelorder[j]->parent;

        levelorder[i]->parent=parentofj;
        levelorder[j]->parent=parentofi;

        if(parentofi->left==levelorder[i] && parentofj->left==levelorder[j])
        {
            parentofi->left=levelorder[j];
            parentofj->left=levelorder[i];
        }
        else if(parentofi->left==levelorder[i] && parentofj->right==levelorder[j])
        {
            parentofi->left=levelorder[j];
            parentofj->right=levelorder[i];
        }
        else if(parentofi->right==levelorder[i] && parentofj->left==levelorder[j])
        {
            parentofi->right=levelorder[j];
            parentofj->left=levelorder[i];
        }
        else if(parentofi->right==levelorder[i] && parentofj->right==levelorder[j])
        {
            parentofi->right=levelorder[j];
            parentofj->right=levelorder[i];
        }


        levelorder=levelOrder();
        for(int i=0; i<levelorder.size()-1; i+=2)
            if(levelorder[i]->parent->weight!=levelorder[i]->weight+levelorder[i+1]->weight)
                levelorder[i]->parent->weight=levelorder[i]->weight+levelorder[i+1]->weight;
        }
    }
    void binstr2file(string x){
        for(int i=0; i<x.size(); i++)
            writebit(x[i]=='1'?1:0, 0, outputfile);
    }

    void code(){
        ifstream original(inputfile, ios::binary);
        char a;
        while(!original.eof()){
            original.read((char*)&a, sizeof(char));
            if(original.eof()) break;
            string code;
            Node *temp;
            if(getNode.find(a)!=getNode.end())
             temp=getNode[a];
            else temp=getNode['\0'];

            while(temp->parent!=nullptr){
                if(temp==temp->parent->left)
                    code+='0';
                else code+='1';
                temp=temp->parent;
                }

            reverse(code.begin(), code.end());

            if(getNode.find(a)==getNode.end()){
                string ascii;
                int c=(int)a;
                for(int i=0; i<8; i++){
                    ascii+=c%2==0?'0':'1';
                    c/=2;
                    }
                reverse(ascii.begin(), ascii.end());
                code.append(ascii);
            }
            update(a);
            binstr2file(code);
        }
        writebit(0,1, outputfile);
    }

     void decode(){
        ifstream mf(inputfile);
        ofstream nf(outputfile);
        mf.seekg(-1, ios::end);
        char padding;
        mf.read((char*)&padding, sizeof(char));
        int i;
        char a=0;
        for(i=0; i<8; i++){
            a*=2;
            if(nextbit(inputfile))
                a+=1;
        }
        update(a);
        nf<<a;
        while(i<((int)mf.tellg()-1)*8-padding){
            Node *temp=root;
            while(temp->left!=nullptr){
                if(!nextbit(inputfile))
                    temp=temp->left;
                else temp=temp->right;
                i++;
            }
            if(temp->c!='\0')
                a=temp->c;
            else {
                a=0;
                for(int j=0; j<8; j++){
                    a*=2;
                    if(nextbit(inputfile))
                        a+=1;
                }
                i+=8;
            }
            update(a);
            nf<<a;
        }
    }

    public:
    Huffman(string file1, string file2, bool mode){
        root=new Node();
        root->c='\0';
        root->weight=0;
        root->parent=nullptr;
        root->left=nullptr;
        root->right=nullptr;

        getNode['\0']=root;

        inputfile=file1;
        outputfile=file2;
        if(mode==0)
            code();
        else decode();
    }
};

int main(){
    string input, output;
    bool mode;
    cout<<"Enter name of the file to be compressed or decompressed.\n";
    cin>>input;
    cout<<"Enter name for compressed or decompressed file you want to create. \n";
    cin>>output;
    cout<<"Enter 0 if you want to compress the file. 1 if you want to decompress the file. \n";
    cin>>mode;
    Huffman h(input, output, mode);
}




#include <iostream>
#include <queue>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

#define NUM_CHARACTERS 256									//The number of ASCII characters

using namespace std;						
int freq_count[NUM_CHARACTERS];								//Used to store the frequencies of each character
string huff_codes[NUM_CHARACTERS];							//Used to store the code word of each character

struct Node{
	char charac;											//The character in a leaf node of the Huffman Tree
	int frequency;											//The frequency of a Node in the Huffman Tree
	Node *left;												//Left child of an internal node. (Not used by leaves)
	Node *right;											//Right child of an internal node. (Not used by leaves)
	
	Node(char c, int f){									//Used when creating a Leaf
		charac = c;
		frequency = f;
		left = NULL;
		right = NULL;
	}

	Node(char c, int f, Node* l, Node* r){					//Used when creating an internal node
		charac = c;
		frequency = f;
		left = l;
		right = r;
	}
};

struct compare_freq{										//Function used to prioritise Nodes in the Huffamn Tree
															//Nodes with the Lower Frequencies are given higher priorities
	bool operator()(const Node* l, const Node* r) const { return (l->frequency > r->frequency); }//inverse of the standard less than comparison
};

void generate_codes(Node *root, string code){				//Function is called recursively. The code gets concatenated with every call
	if (root->left == NULL && root->right == NULL)			//When leaf is reached, code gets written to the array for the specified character at that leaf
		huff_codes[root->charac] = code;
	if (root->left != NULL)									//If node is internal, move to left child and add '0' to the code
		generate_codes(root->left, code + "0");
	if (root->right != NULL)								//If node is internal, move to right child and add '0' to the code
		generate_codes(root->right, code + "1");
}

Node* build_Tree(){														//Function used to build the Huffman Tree
	priority_queue<Node*, vector<Node*>, compare_freq> init_Huff_tree;	//Queue used to store Nodes
																		//Sorts each Node in ascending order of frequencies (determined by function "compare_freq")
	for (int i = 0; i < NUM_CHARACTERS; i++)
		if (freq_count[i] != 0)											//For every character with a frequency greater than zero,
			init_Huff_tree.push(new Node((char)i, freq_count[i]));		//Create a leaf Node for that character

	while (init_Huff_tree.size() > 1){									//Stops when there is one Node left in the queue, this is the root node of the tree
		Node* tempL = init_Huff_tree.top();								//Get the 2 nodes with the lowest frequencies
		init_Huff_tree.pop();											//Remove them from the queue
		Node* tempR = init_Huff_tree.top();
		init_Huff_tree.pop();
																		//Create a parent Node with the extracted Nodes as child Nodes and a frequecy equal to 
																		//the combined frequencies of the child nodes. Add this node to the queue
		init_Huff_tree.push(new Node(NULL, (tempL->frequency + tempR->frequency), tempL, tempR));
	}
	return init_Huff_tree.top();
}

short bin_to_base10(string bin){							//Function used to convert a binary number (stored as a string), to
															//its base 10 version of it (stored as a short)
	short ans = 0;

	for (int i = 0; i < bin.length(); i++)
		ans += ((bin[i] - '0') * pow(2, (bin.length() - i - 1)));

	return ans;
}

string base10_to_bin(int n){								//Function used to convert a base 10 number (stored as an integer), yo
															//its 8-bit binary representation of it (stored as string)
	string ans = "";	
	
	while (n != 0){
		ans = to_string((n % 2)) + ans;
		n = n / 2;
	}
	stringstream ss;
	ss << setw(8) << setfill('0') << ans;					//Converting Integer to string and adding zeros to the MSBs to ensure it is 8 bits
	ans = ss.str();
	return ans;
}

void encode(string text){									//Function used to encode/ compress text
	
	for (int i = 0; i < text.length(); i++)					//Goes throught every character in the text and increments its frequency which is stored
		freq_count[text[i]]++;								//in the freq_count array

	Node* root = build_Tree();								//Builds the Huffman tree
	generate_codes(root, "");								//Generates the codes for all characters found in the text
	
	string out_string = "";
	for (int i = 0; i < text.size(); i++)					//Matches characters in text with their codes and replaces them in out_string
		out_string += huff_codes[(text[i] < 0) ? (text[i] + 256):(text[i])];

	while (out_string.size() % 8 != 0)						//Ensures out_string's lenght is divisble by 8
		out_string += "0";

	string out_ASCII = "";									//Text to write to output file
	for (int i = 0; i < out_string.length(); i = i + 8)		//Takes 8 characters at a time from out_string (this acts as a byte since it can either be a 1 or 0)
	{														//Convert that byte to an integer using bin_to_base10
		out_ASCII += bin_to_base10(out_string.substr(i, 8));//Store the ASCII character of that integer to out_ASCII (this will be written to text file)
	}

	ofstream outfile("Encoded_output.txt");
	char spacer = 254;										//Character used as delimiter
	for (int i = 0; i < NUM_CHARACTERS; i++)				//Write frequency table to output, add delimiter inbetween each frequency
		outfile << freq_count[i] << spacer;
	outfile << out_ASCII;									//Write out_ASCII(which is the final compressed version of the original text) to text file
	outfile.close();
	cout << "Encoded text written to Encoded_output.txt" << endl;
}

void decode(string text){									//Function used to decode/ decompress text
	
	int pos = 0 , k = 0;									//Used as counter variables
	const char c = 254;										//Delimiter character
	string tmp = "";
	
	while (pos < NUM_CHARACTERS){							//Stops when all frequencies are recorded
		int t = text.find(c);								//Finds first delimiter
		tmp = text.substr(k, t);							//Gets frequency and stores in tmp as string
		k = t;
		text = text.substr(k+1, text.length());				//Removes the frequency and delimiter that was read
		freq_count[pos] = stoi(tmp);						//Converts tmp to integer and stores in freq_count array
		k = 0;												//Starts from the beginning of the text
		pos++;												//Move to next character in frequency array
	}

	string decoded_ASCII;									//Used to store the binary representation of the compressed text
	string decoded_text;									//Used to store the decompressed text
	
	for (int i = 0; i < text.length(); i++){				//Traverse every character in the compressed text. The aim is to get the binary
		if ((int)text[i] < 0)								//representation of these characters
			decoded_ASCII += base10_to_bin(text[i] + 256);	//Call base10_to_binary to convert. Store in decoded_ASCII.
		else
			decoded_ASCII += base10_to_bin(text[i]);
	}
	
	Node* root = build_Tree();								//Build the Huffman Tree
	Node* next = root;										//Auxilary pointer
		
	k = 0;
	while (k <= decoded_ASCII.length()){					//Traverse every "bit" of decoded_ASCII
		if (next->left == NULL && next->right == NULL){		//Stops when leaf node is reached. Then restart from root node
			decoded_text += next->charac;					//Store character from leaf node to decoded_text
			next = root;
		}
		else if (decoded_ASCII[k] == '0'){					//Move to left child node if '0' is read
			next = next->left;								
			k++;											//Go to next character in decoded_ASCII
		}
		else{
			next = next->right;								//Move to right child node if '1' is read
			k++;											//Go to next character in decoded_ASCII
		}
	}

	ofstream outfile("Decoded_output.txt");
	outfile << decoded_text;								//Write decompressed text to file
	outfile.close();
	cout << "Decoded text written to Decoded_output.txt" << endl;
}

int main(){
	string file_name, document_text;
	bool encode_bool;
	
	for (int i = 0; i < NUM_CHARACTERS; i++)				//Initialize global arrays
		freq_count[i] = 0;
	for (int i = 0; i < NUM_CHARACTERS; i++)
		huff_codes[i] = "";
	
	cout << "Enter the file name : ";						//Gets file name from user
	cin >> file_name;

	ifstream file(file_name, ios::binary);					//Opens file in binary mode
	
	if(file.is_open()){
		auto ss = ostringstream{};
		ss << file.rdbuf();
		document_text = ss.str();							//Reads whole file and store in string variable document_text
	}
	else{
		cout << "Error Reading file" << endl;
		system("pause");									//Pauses the screen
		return 0;
	}

	file.close();

	cout << "What would you like to do to the file (1 = Compress, 0 = Decompress) ? ";
	cin >> encode_bool;										//Asks user if a compression or decompression must be done

	const clock_t begin = clock();							//Clock used to measure time used for Compression/ Decompression

	if (encode_bool)
		encode(document_text);
	else
		decode(document_text);

	
	cout << "Time taken for operation : " << setprecision(10) <<float(clock() - begin) / CLOCKS_PER_SEC << endl;
	system("pause");;										//Pauses Screen
	return 0;
}
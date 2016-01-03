#include <sstream> // stringstream.
#include <iostream> // cout, cerr.
#include <fstream> // ifstream.
#include <istream> //istream.
using namespace std;

class Matrix //matrix class.
{
public:
	Matrix(int sizeR, int sizeC, double* input_data); //Matrix constructor.
	Matrix(const Matrix& cMatrix); //Matrix copy constructor.
	virtual Matrix::~Matrix(); //Matrix destructor (made virtual so that the inherited class data can be deleted.
	Matrix Matrix::operator+(const Matrix& B); // + operator overloader.
	Matrix Matrix::operator-(const Matrix& B); // - operator overloader.
	Matrix Matrix::operator*(const Matrix& B); // * operator overloader.
	Matrix& Matrix::operator=(const Matrix& B); // = operator overloader.
	double Matrix::operator()(int a, int b); //subcript operator overloader.
	double* getData() { return data; }; //function returns a pointer to data.
	double get(int i, int j)const { return data[i*N + j]; } //const goes here to ensure the values cannot be changed by the function. 
	double* getBlock(int sr, int er, int sc, int ec); //function gets sub block using start row, end row, start column, end column.
	void set(int i, int j, double input_data) { data[i*N + j] = input_data; } //sets a block of data with the input data.
protected: //protected so inherited members can access but still private.
	int M; //used to store the amount of columns.
	int N; //used to store the amount of rows.
	double* data; //a pointer to where the data is stored.
};

class BinaryImage : public Matrix //binaryImage class inherited from Matrix class.
{
public:
	BinaryImage(int M, int N, double* input_data, double thresh); //BinaryImage constructor.
	BinaryImage(const BinaryImage& cBinaryImage); //BinaryImage copy constructor.
	~BinaryImage(); //binaryImage destructor.
};

double* readTXT(char *fileName, int sizeR, int sizeC); //function for taking and storing a text file.
void WritePGM(char *filename, double *data, int sizeR, int sizeC, int Q); //function for outputting to .PGM format.

int main()
{
	int M = 512; //size of the grids pixel width (col).
	int N = 512; //size of the grids pixel height (row).
	int threshold = 175; //used to set the threshold of the binary conversion.
	int blockPixAmt = 32; //stores the amount of pixels in each block.
	int blockAmt = 16; //stores the amount of blocks in each row/column.
	double SSD[256]; //array used to store each sum of squared value for each block.
	double sumValue = 0; //used to add each sum of squared value together for each block.

	//location values to grab pixel values from A (shuffled image).
	int shuffled_srow = 0; //start row.
	int shuffled_erow = 31; //end row.
	int shuffled_scol = 0; //start column.
	int shuffled_ecol = 31; //end column.
	//location values to grab pixel values from B (noised image)
	int noised_srow = 0; //start row.
	int noised_erow = 31; //end row.
	int noised_scol = 0; //start column.
	int noised_ecol = 31; //end column.

	char* inputFileName = "./Input/logo_shuffled.txt"; //stores the location of the shuffled logo text file.
	double* input_data = 0;	 //used to store the input shuffled text file
	input_data = readTXT(inputFileName, M, N); //function takes text file as input and stores inside input_data.
	char* inputNoiseFileName = "./Input/logo_with_noise.txt"; //stores the location of the noised logo text file.
	double* inputNoise_data = 0; //used to store the input noised text file
	inputNoise_data = readTXT(inputNoiseFileName, M, N); //function takes text file as input and stores inside inputNoise_data.

	cout << endl << "Data from text file -------------------------------------------" << endl; //displays message to user.

	BinaryImage originalShuffled(M, N, input_data, threshold); //Binary image object created storing the original SHUFFLED image.
	BinaryImage originalNoised(M, N, inputNoise_data, threshold); //Binary image object created storing the NOISED image.

	double* output_data = new double[M*N]; //used to store the data of the final unshuffled image.
	Matrix finalImage(M, N, output_data); //used to store the final unshuffled image for output.

	for (int xx = 0; xx<blockAmt; xx++) //counts through 16 blocks (rows).
	{
		for (int yy = 0; yy<blockAmt; yy++) //counts through 16 blocks (columns).
		{
			int ssdCount = 0; //used to store the current place in the SSD array.
			Matrix smallShuffled((blockPixAmt), (blockPixAmt), originalShuffled.getBlock(shuffled_srow, shuffled_erow, shuffled_scol, shuffled_ecol)); //32x32 matrix 'smallShuffled' created from current section of SHUFFLED image.
			for (int ii = 0; ii<blockAmt; ii++) //loop goes through each vertical block beginning top to bottom.
			{
				for (int jj = 0; jj<blockAmt; jj++) //loop goes through each horizontal block beginning at the left to right.
				{
					Matrix smallNoised((blockPixAmt), (blockPixAmt), originalNoised.getBlock(noised_srow, noised_erow, noised_scol, noised_ecol)); //32x32 matrix 'smallNoised' created from current section of NOISED image.
					Matrix smallSumSquared = smallShuffled - smallNoised; //Matrix smallSumSquared created by subtracting smallShuffled from smallNoised.
					smallSumSquared = smallSumSquared*smallSumSquared; //all values in smallSumSquared are multiplied by themselves to get the sum of squared.
					for (int kk = 0; kk<blockPixAmt - 1; kk++) //workout the sum of all values (horizontally).
					{
						for (int ll = 0; ll<blockPixAmt - 1; ll++) //workout the sum of all values (vertically).
						{
							{
								sumValue = sumValue + smallSumSquared.get(kk, ll); //retrieve each value from smallSumSquared and sum them.
							}
						}
					}
					SSD[ssdCount] = sumValue; //store each summed value in an array.
					sumValue = 0;  //reset sum sumValue to 0.
					ssdCount++; //add one to ssdCount so as to store in next location of SSD array.
					noised_scol = noised_ecol + 1; //move the start location of the column to the start of the next block of 32.
					noised_ecol = noised_ecol + 32; //move the end location of the column to the end of the next block of 32.
				}
				noised_scol = 0; //reset column start to 0 to start again on the next row of 32.
				noised_ecol = 31; //reset column end to 31 to start again on the next row of 32.
				noised_srow = noised_erow + 1; //move the start location of the row to the start of the next column of 32.
				noised_erow = noised_erow + 32; //move the end location of the row to the end of the next column of 32.
			}
			//the following code finds the index of the closest matched block.	
			int index = 0;
			int smallestIndex = 0;
			for (index = smallestIndex; index < 256; index++) //work out the index of the smallest sumValue in array.
			{
				if (SSD[smallestIndex] > SSD[index]) //if the size of index is less than smallestIndex then swap them.
				{
					smallestIndex = index;
				}
			}
			int r = smallestIndex / 16 * 32; //works out the pixel distance of 'originalNoised' from the top.
			int c = smallestIndex % 16 * 32; //works out the pixel distance of 'originalNoised' from the left.
			for (int kk = 0; kk<32; kk++) //moves through the row blocks.
			{
				for (int ll = 0; ll<32; ll++) //moves through the columns blocks.
				{
					finalImage.set(r + kk, c + ll, originalShuffled.get(shuffled_srow + kk, shuffled_scol + ll)); //finalImage stores the value of originalShuffled into the correct location.
					originalNoised.set(r + kk, c + ll, 5); //set each read section of A to 5 so the block is not used again.
				}
			}
			//pixel locations of the originalNoised columns and rows are reset.
			noised_scol = 0;
			noised_ecol = 31;
			noised_srow = 0;
			noised_erow = 31;
			//pixel locations of the originalShuffled columns are moved by 32.
			shuffled_scol = shuffled_ecol + 1;
			shuffled_ecol = shuffled_ecol + 32;
		}
		//pixel locations of the originalShuffled columns are reset.
		shuffled_scol = 0;
		shuffled_ecol = 31;
		//pixel locations of the originalShuffled rows are moved by 32.
		shuffled_srow = shuffled_erow + 1;
		shuffled_erow = shuffled_erow + 32;
	}

	//the following code checks if the pixel is 1 or 0. If 1 it converts to 255 (black and white).
	for (int i = 0; i<M; i++)
	{
		for (int j = 0; j<N; j++)
		{
			if (finalImage.get(i, j) == 1)
			{
				finalImage.set(i, j, 255);
			}
			else
			{
				finalImage.set(i, j, 0);
			}
		}
	}
	output_data = finalImage.getData(); //stores the data from final image into output_data for exporting.
	//the following code restores the image to the file location below.
	char* outputFileName = "./Output/logo_restored.pgm";
	int Q = 255;
	WritePGM(outputFileName, output_data, M, N, Q); //function to restore data to PGM format.
	delete[] input_data; //deletes input_data array.
	return 0;
}

double* readTXT(char *fileName, int sizeR, int sizeC) //used to read in the text file and store into an array.
{
	double* data = new double[sizeR*sizeC];
	int i = 0;
	ifstream myfile(fileName);
	if (myfile.is_open())
	{
		while (myfile.good())
		{
			if (i>sizeR*sizeC - 1) break;
			myfile >> *(data + i);
			i++;
		}
		myfile.close();
	}
	else cout << "Unable to open file";
	return data;
}
void WritePGM(char *filename, double *data, int sizeR, int sizeC, int Q) //used to write the finished image data into a PGM format.
{
	int i;
	unsigned char *image;
	ofstream myfile;
	image = (unsigned char *) new unsigned char[sizeR*sizeC];
	for (i = 0; i<sizeR*sizeC; i++)
		image[i] = (unsigned char)data[i];
	myfile.open(filename, ios::out | ios::binary | ios::trunc);
	if (!myfile) {
		cout << "Can't open file: " << filename << endl;
		exit(1);
	}
	myfile << "P5" << endl;
	myfile << sizeC << " " << sizeR << endl;
	myfile << Q << endl;
	myfile.write(reinterpret_cast<char *>(image), (sizeR*sizeC)*sizeof(unsigned char));
	if (myfile.fail()) {
		cout << "Can't write image " << filename << endl;
		exit(0);
	}
	myfile.close();
	delete[] image;
}
double* Matrix::getBlock(int sr, int er, int sc, int ec) //function returns a block from matrix.
{
	int rowsize = er - sr + 1; //works out the row size.
	int colsize = ec - sc + 1; //works out the column size.
	int counter = 0; //used to count through the array.
	double *smallBlock = new double[rowsize*colsize]; //creates memory for smallblock array.
	//the code below retrieves from the correct location of data and stores in smallblock.
	for (int i = sr; i <= er; i++)
	{
		for (int j = sc; j <= ec; j++)
		{
			smallBlock[counter] = data[i*N + j];
			counter++;
		}
	}
	return smallBlock; //smallblock is returned.
}
Matrix::Matrix(int sizeR, int sizeC, double* input_data) //Matrix object constructor. Used when building any matrix objects.
{
	M = sizeR;
	N = sizeC;
	data = new double[sizeR*sizeC];
	for (int ii = 0; ii<sizeR*sizeC; ii++)
	{
		data[ii] = input_data[ii];
	}
}
Matrix::~Matrix() //Matrix object destructor. Used to destroy any matrix data when out of scope.
{
	delete[] data;
}
Matrix::Matrix(const Matrix& cMatrix) //Matrix copy constructor. Used when copying any matrix objects.
{
	M = cMatrix.M;
	N = cMatrix.N;
	data = new double[M*N];
	for (int i = 0; i <M*N; i++)
	{
		data[i] = cMatrix.data[i]; //copys from the original to the new version.
	}
}

//The following are overloaded operators for the Matrix class.
Matrix Matrix::operator+(const Matrix& B) // + operator overloader.
{
	double *newData = new double[M*N];
	int kk = 0;
	for (int ii = 0; ii<M; ii++)
	{
		for (int jj = 0; jj<N; jj++)
		{
			newData[kk] = data[ii*N + jj] + B.get(ii, jj); //adds the original to the new version.
			kk++;
		}
	}
	return Matrix(M, N, newData);
}
Matrix Matrix::operator-(const Matrix& B) //minus operator overloader.
{
	double *newData = new double[M*N];
	int kk = 0;
	for (int ii = 0; ii<M; ii++)
	{
		for (int jj = 0; jj<N; jj++)
		{
			newData[kk] = data[ii*N + jj] - B.get(ii, jj); //subtracts the new version from the original copy.
			kk++;
		}
	}
	return Matrix(M, N, newData);
}
Matrix Matrix::operator*(const Matrix& B) // * operator overloader
{
	double *newData = new double[M*N];
	int kk = 0;
	for (int ii = 0; ii<M; ii++)
	{
		for (int jj = 0; jj<N; jj++)
		{
			newData[kk] = data[ii*N + jj] * B.get(ii, jj); //multiplies the new version with the original copy.
			kk++;
		}
	}
	return Matrix(M, N, newData);
}
Matrix& Matrix::operator=(const Matrix& B) // = operator overloader
{
	if (this == &B)
	{
		return *this;
	}
	else
	{
		delete[] data; //deletes anything stored in data
		M = B.M;
		N = B.N;
		data = new double[M*N];
		for (int ii = 0; ii<M; ii++)
		{
			for (int jj = 0; jj<N; jj++)
			{
				data[ii*N + jj] = B.get(ii, jj);	//copies data from original to copy.
			}
		}
		return *this;
	}
}
double Matrix::operator()(int a, int b) // subscript operator overloader
{
	int i = a;
	int j = b;
	return data[i*N + j]; //returns the data at the location.
}

BinaryImage::BinaryImage(int M, int N, double* input_data, double thresh) :Matrix(M, N, input_data) //the binary image constructor.
{
	//When created changes any number in the array lower than the threshold to 0 and higher to 1.
	for (int i = 0; i<M; i++)
	{
		for (int j = 0; j<N; j++)
		{
			if (input_data[i*N + j]>thresh)
			{
				data[i*N + j] = 1;
			}
			else
			{
				data[i*N + j] = 0;
			}
		}
	}
}
BinaryImage::BinaryImage(const BinaryImage& cBinaryImage) :Matrix(cBinaryImage) //the binary image copy constructor
{
	M = cBinaryImage.M;
	N = cBinaryImage.N;
	data = new double[M*N];
	for (int i = 0; i <M*N; i++)
	{
		data[i] = cBinaryImage.data[i]; //copies data from the original to the copy.
	}
}
BinaryImage::~BinaryImage() //BinaryImage destructor
{
	delete[] data;
	data = 0;
}
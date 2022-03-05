#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector> 
#include <sys/stat.h>

/* This code is derived in parts from LZW@RosettaCode for UA CS435 
*/ 
 
// Compress a string to a list of output symbols.
// The result will be written to the output iterator
// starting at "result"; the final iterator is returned.

template <typename Iterator>
Iterator compress(const std::string &uncompressed, Iterator result) {
  // Build the dictionary, start with 256.
  int dictSize = 256;
  std::map<std::string,int> dictionary;
  for (int i = 0; i < 256; i++)
    dictionary[std::string(1, i)] = i;
 
  std::string w;
  for (std::string::const_iterator it = uncompressed.begin();
       it != uncompressed.end(); ++it) {
    char c = *it;
    std::string wc = w + c;
    if (dictionary.count(wc))
      w = wc;
    else {
      *result++ = dictionary[w];
      // Add wc to the dictionary. Assuming the size is 4096!!!
      if (dictionary.size()<4096)
         dictionary[wc] = dictSize++;
      w = std::string(1, c);
    }
  }
 
  // Output the code for w.
  if (!w.empty())
    *result++ = dictionary[w];
  return result;
}
 
// Decompress a list of output ks to a string.
// "begin" and "end" must form a valid range of ints
template <typename Iterator>
std::string decompress(Iterator begin, Iterator end) {
  // Build the dictionary.
  int dictSize = 256;
  std::map<int,std::string> dictionary;
  for (int i = 0; i < 256; i++)
    dictionary[i] = std::string(1, i);

  std::string w(1, *begin++);
  std::string result = w;
  //std::cout << "\ndecompressed: " << result <<";";
  std::string entry;
  for ( ; begin != end; begin++) {
    int k = *begin;
    if (dictionary.count(k))
      entry = dictionary[k];
    else if (k == dictSize)
      entry = w + w[0];
    else
      throw "Bad compressed k";

    result += entry;
    //std::cout << "\ndecompressed: " << result <<";";

    // Add w+entry[0] to the dictionary.
    if (dictionary.size()<4096)
      dictionary[dictSize++] = w + entry[0];

    w = entry;
  }
  return result;
}

//
std::string int2BinaryString(int c, int cl) {
      std::string p = ""; //a binary code string with code length = cl
      int code = c;
      while (c>0) {         
		   if (c%2==0)
            p="0"+p;
         else
            p="1"+p;
         c=c>>1;   
      }
      int zeros = cl-p.size();
      if (zeros<0) {
         std::cout << "\nWarning: Overflow. code " << code <<" is too big to be coded by " << cl <<" bits!\n";
         p = p.substr(p.size()-cl);
      }
      else {
         for (int i=0; i<zeros; i++)  //pad 0s to left of the binary code if needed
            p = "0" + p;
      }
      return p;
}

//
int binaryString2Int(std::string p) {
   int code = 0;
   if (p.size()>0) {
      if (p.at(0)=='1') 
         code = 1;
      p = p.substr(1);
      while (p.size()>0) { 
         code = code << 1; 
		   if (p.at(0)=='1')
            code++;
         p = p.substr(1);
      }
   }
   return code;
}

void binaryIODemo(std::vector<int> compressed) {
   //example for integer c; looking for a 9 bit code
   int c = 69;
   int bits = 9; //length of the code
   std::string p = int2BinaryString(c, bits);
   std::cout << "c=" << c <<" : binary string="<<p<<"; back to code=" << binaryString2Int(p)<<"\n";
   
   std::string bcode= "";
   for (std::vector<int>::iterator it = compressed.begin() ; it != compressed.end(); ++it) {
   
      if (*it<256)
         bits = 8; 
      else
         bits = 9;
      
      //assuming 12 bits
      bits = 12;
      p = int2BinaryString(*it, bits);
      std::cout << "c=" << *it <<" : binary string="<<p<<"; back to code=" << binaryString2Int(p)<<"\n";
      bcode+=p;
   }
   
   //writing to file
   std::cout << "string 2 save : "<<bcode << "\n";
   std::string fileName = "example435.lzw"; //this is for demo you should not hard code the final name.
   std::ofstream myfile;
   myfile.open(fileName.c_str(),  std::ios::binary);
   
   std::string zeros = "00000000";
   if (bcode.size()%8!=0) //make sure the length of the binary string is a multiple of 8
      bcode += zeros.substr(0, 8-bcode.size()%8);
   
   int b; 
   for (int i = 0; i < bcode.size(); i+=8) { 
      b = 1;
      for (int j = 0; j < 8; j++) {
         b = b<<1;
         if (bcode.at(i+j) == '1')
           b+=1;
      }
      char c = (char) (b & 255); //save the string byte by byte
      myfile.write(&c, 1);  
   }
   myfile.close();
   
   //reading from a file
   std::ifstream myfile2;
   myfile2.open (fileName.c_str(),  std::ios::binary);
   
   struct stat filestatus;
   stat(fileName.c_str(), &filestatus );
   long fsize = filestatus.st_size; //get the size of the file in bytes
   
   char c2[fsize];
   myfile2.read(c2, fsize);
   
   std::string s = "";
   long count = 0;
   while(count<fsize) {
      unsigned char uc =  (unsigned char) c2[count];
      std::string p = ""; //a binary string
      for (int j=0; j<8 && uc>0; j++) {         
		   if (uc%2==0)
            p="0"+p;
         else
            p="1"+p;
         uc=uc>>1;   
      }
      p = zeros.substr(0, 8-p.size()) + p; //pad 0s to left if needed
      s+= p; 
      count++;
   } 
   myfile2.close();
   std::cout << " saved string : "<<s << "\n"; 
}
 
// demo of how LZW works

int main(int argc, char *argv[]) {
   if((argc != 3 || argv[1][0]!='c') && (argc !=3 || argv[1][0]!='e')){
      std::cout << "Wrong format. \nFor Compression of a file, please enter: \n lzw435 c filaname \n";
      std::cout << "For expansion of a file, please enter: \n lzw435 e filename \n"; 
   } else {
      if(argv[1][0] == 'c'){

         //Here we need to compress a given file
         //Our first task is to read the input from the compressed file
         std::string fileName = argv[2];
         std::ifstream inputFile;
         inputFile.open(fileName.c_str());
   
         //Get the size of a file in bytes
         struct stat fileStatus;
         stat(fileName.c_str(), &fileStatus );
         long fileSize = fileStatus.st_size;
   
         //Read in the file byte by byte
         /*char characterArray[fileSize];
         inputFile.read(characterArray, fileSize);  */
         std::string fileContent = std::string((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
   
         //close the input file
         inputFile.close();

         //Test to see if we read in the values correctly
         std::cout << "Input String: " << fileContent << "\n";

         //Now we need to compress the input
         std::vector<int> compressed;
         compress(fileContent, std::back_inserter(compressed));

         //The compressed should be a vector of ints representing the input. 
         //We now need to convert those to binary strings.
         std::string currentBString = "";
         std::string totalBString = "";
         //asumme 12 bits
         int bits = 12;
          for(auto itr=compressed.begin(); itr !=compressed.end(); ++itr){
               currentBString = int2BinaryString(*itr, bits);
               std::cout << "c=" << *itr <<" : binary string="<<currentBString<<"; back to code=" << binaryString2Int(currentBString)<<"\n";
               //append it to a final string
               totalBString += currentBString;
          }
         //test to see if the total string is binary
         std::cout << "Total Binary String: " << totalBString << "\n";

         //now we need to write this binary string to an output file
         std::string outputName = fileName + ".lzw";
         std::ofstream outputFile;
         outputFile.open(outputName.c_str(),  std::ios::binary);

         //make sure the length of the binary string is a multiple of 8
         std::string zeros = "00000000";
         if(totalBString.size() % 8 != 0){
            totalBString += zeros.substr(0, 8-totalBString.size()%8);
         }

         std::cout << "Binary String as Multiple of 8: " << totalBString << "\n";
         int b; 
         for (int i = 0; i < totalBString.size(); i+=8) { 
            b = 1;
            for (int j = 0; j < 8; j++) {
               b = b<<1;
               if (totalBString.at(i+j) == '1')
                  b+=1;
            }
            char c = (char) (b & 255); //save the string byte by byte
            outputFile.write(&c, 1);  
         } 
         outputFile.close();

      }else if (argv[1][0] == 'e'){
         try 
         {
            //Here we need to expand the given file
            //create a file to read from given the input filename
            std::string inputFileName = argv[2];
            std::ifstream inputFile;
            inputFile.open(inputFileName.c_str(), std::ios::binary);

            //get the size of the file in bytes
            struct stat fileStatus;
            stat(inputFileName.c_str(), &fileStatus);
            long fileSize = fileStatus.st_size;

            //read in the contents into a character array
            char characterArray[fileSize];
            inputFile.read(characterArray, fileSize);

            //test to see if you read the contents in correctly
            //std::cout << "Input file contents: " << characterArray << "\n";

            //check to see if they are corrent binary strings
            std::string zeros = "00000000";
            std::string totalBinaryString = "";
            int counter = 0;
            while(counter < fileSize){
               unsigned char currentChar = (unsigned char) characterArray[counter];
               std::string currentBString = "";
               for(int j = 0; j < 8 && currentChar > 0; j++){
                  if(currentChar % 2 == 0){
                     currentBString = "0" + currentBString;
                  }else{
                     currentBString = "1" + currentBString;
                  }
                  currentChar = currentChar >> 1;
               }
               //pad 0s to left if needed
               currentBString = zeros.substr(0, 8-currentBString.size()) + currentBString;
               totalBinaryString += currentBString;
               counter++;
            }

            //test to see if the contents are trule a binary string
            std::cout << "Total Binary String: " << totalBinaryString << "\n";

            //close the input file
            inputFile.close();

            //now store 12 chars of the binary string into elements of an int vector
            std::vector<int> compressed;
            for(int i = 0; i < totalBinaryString.size(); i+=12){
               auto smallBString = totalBinaryString.substr(i, 12);
               int currentInt = binaryString2Int(smallBString);
               if(currentInt != 0){
                  compressed.push_back(currentInt);
               }
               //compressed.push_back(binaryString2Int(smallBString));
               //remove padding
               //i = i + 4;
            }
         
            //so now compressed should hold integers values to represent twelve bits of the binary string
            //test to see if its true
         
            /*std::cout << "Compressed: " << "\n";
            for(auto itr=compressed.begin(); itr !=compressed.end(); itr++){
               std::cout<<"\n"<<*itr;
            }  */

            // Now that we have the compressed contents, we need to decompress it.
            std::string finalString = decompress(compressed.begin(), compressed.end());

            // Test to see if it is truly the final String
            //std::cout << "\n" << "Final String: " << finalString << "\n"; 

            // Now we need to write this to the output file
            std::string outputFileName = inputFileName.substr(0, inputFileName.size() - 8).append("2.txt");
            //test to see if its in the format of "filename2"
            std::cout << "Output Filename: " << outputFileName;
            //create output file
            std::ofstream outputFile(outputFileName);
            //write to the file
            outputFile << finalString;

            //close the file
            outputFile.close(); 

         } 
         catch(char const* e)
         {
            std::cerr << e << '\n';
         }
      }
   }

}
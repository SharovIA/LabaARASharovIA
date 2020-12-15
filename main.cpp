#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <windows.h>
#include <chrono>
#include <sstream>
#include<fstream>
#include<random>
#include <iomanip>
using std::vector;
using std::string;
using std::cout;
using std::endl;
using namespace std::chrono;

//KMP
void prefix_function(vector<int> &pi, string &pattern)
{
    size_t lengthPattern = pattern.length();
    pi[0] = 0;
    for (int i = 1; i < lengthPattern; ++i) {
        int j = pi[i - 1];

        while ((j > 0) && (pattern[i] != pattern[j]))
            j = pi[j - 1];

        if (pattern[i] == pattern[j])
            ++j;

        pi[i] = j;
    }
}

void seachKMP(string &text, string &pattern) {

    high_resolution_clock::time_point start = high_resolution_clock::now();

        vector<int> result;
        int lengthText = text.size();
        int lengthPattern = pattern.size();
        vector<int> pi(lengthPattern);

        prefix_function(pi, pattern);

        int position = 0;
        for (int i = 0; i < lengthText; i++) {
            while (position == lengthPattern || (position > 0 && pattern[position] != text[i])) {
                position = pi[position - 1];
                if (lengthPattern - position > lengthText - i)
                    break;
            }

            if (text[i] == pattern[position]) {
                position++;
            }

            if (position == lengthPattern) {
                result.push_back(i - position + 1);
            }

        }

        high_resolution_clock::time_point end = high_resolution_clock::now();
        duration<double>  timeRun = duration_cast<duration<double>>(end - start);

        cout << "KMP алгоритм нашел в тексте " << result.size() << " вхождений" << endl;
        cout << "Время работы KMP алгоритма: " << std::fixed << timeRun.count() << " секунд" << endl;
        cout << "" << endl;
}

//PK
//Быстрое возведение в степень
unsigned long long powerFast(unsigned long long a, unsigned long long n, const unsigned long long p) {
    unsigned long long result = 1;

    while (n) {
        if (n % 2 == 0) {
            n /= 2;
            a = (a * a) % p;
        }
        else {
            n--;
            result = (result * a) % p;
        }
    }

    return result;
}

void getPows(vector<unsigned long long>& aPows, vector<unsigned long long>& aNegativePows, const unsigned long long a, const unsigned long long p) {
    // считаем все степени a 
    aPows[0] = 1;
    aNegativePows[0] = 1; //для упрощения отриц.степений и соответствия их иедексам: первый элемент этого вектора равен a^0 =1
    // 1/a = a^{p-2} mod p
    const unsigned long long aNegative = powerFast(a, p - 2, p);
    for (size_t i = 1; i < aPows.size(); ++i) {
        aPows[i] = (aPows[i - 1] * a) % p;
        aNegativePows[i] = (aNegativePows[i - 1] * aNegative) % p;
    }
}

void getHashOfText(vector<unsigned long long>& aPows, string& text, vector<unsigned long long>& hashPrefixOfText, const unsigned long long a, const unsigned long long p) {
    //// считаем хэши от всех префиксов текста T
    hashPrefixOfText[0] = 0;
    for (size_t i = 1; i <= text.length(); ++i) {
        hashPrefixOfText[i] = (hashPrefixOfText[i - 1] + ((unsigned long long)(text[i - 1] + 128) * aPows[i - 1]) % p) % p;
    }
}

unsigned long long getHashOfPattern(vector<unsigned long long>& aPows, string& pattern, const unsigned long long a, const unsigned long long p) {
    // считаем хэш образца
    unsigned long long hashOfPattern = 0;
    for (size_t i = 0; i < pattern.length(); ++i)
        hashOfPattern = (hashOfPattern + (pattern[i] + 128) * aPows[i] % p) % p;
    return hashOfPattern;
}

unsigned long long getCurrentHash(int i, int j, vector<unsigned long long>& hashsFromPrefixOfText, vector<unsigned long long>& aNegativePows, const unsigned long long p) {
    return (((hashsFromPrefixOfText[j] - hashsFromPrefixOfText[i] + p) % p) * aNegativePows[i]) % p;
}

void rabinKarpSearch(string &text, string &pattern) {

        high_resolution_clock::time_point start = high_resolution_clock::now();

        vector<int> result;
        //p - простое
        const unsigned long long p = 2147483647;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<long long> distribution(0, p - 1);
        //a - случайное из [0, p-1]
        const unsigned long long a = distribution(gen);
        vector<unsigned long long> aPows(text.length());
        vector<unsigned long long> aNegativePows(text.length());
        vector<unsigned long long> hashsFromPrefixOfText(text.length() + 1);
        unsigned long long hashOfPattern;

        //подготовка
        getPows(aPows, aNegativePows, a, p);
        getHashOfText(aPows, text, hashsFromPrefixOfText, a, p);
        hashOfPattern = getHashOfPattern(aPows, pattern, a, p);
      
       // основной алгоритм
        size_t lengthPattern = pattern.length();
        size_t countSdvigs = text.length() - pattern.length() + 1;
        unsigned long long indexInPattern;
        bool isCheckSubStr = true;
        unsigned long long current;

        for (size_t i = 0; i < countSdvigs; ++i) {

            current = getCurrentHash(i, i + lengthPattern, hashsFromPrefixOfText, aNegativePows, p);

            if (hashOfPattern == current) {
                //хеши совпали, проверяем посимвольно
                indexInPattern = 0;
                isCheckSubStr = true; //вроде нашли подстроку

                for (size_t j = i; j < i + pattern.length(); ++j, ++indexInPattern) {
                    if (text[j] != pattern[indexInPattern]) {
                        //подстрока не совпала с образцом
                        isCheckSubStr = false;
                        break;
                    }
                }

                if (isCheckSubStr)
                    result.push_back(i);
            }
        }

        high_resolution_clock::time_point end = high_resolution_clock::now();
        duration<double>  timeRun = duration_cast<duration<double>>(end - start);

        cout << "PK алгоритм нашел в тексте " << result.size() << " вхождений" << endl;
        cout << "Время работы PK алгоритма: " << std::fixed << timeRun.count() << " секунд" << endl;
        cout << "" << endl;
}

//наивный поиск
void search(string text, string pattern) {
   
    high_resolution_clock::time_point start = high_resolution_clock::now();

    vector<int> result;
    size_t lengthPattern = pattern.length();
    size_t countSdvigs = text.length() - pattern.length()+1;
    
    for (long long i = 0; i < countSdvigs; i++) {

        long long j = 0;

        for (j; (pattern[j] == text[i + j] && j < lengthPattern); j++);

        if (j == pattern.length()) {
            result.push_back(i);
        }
    }

    high_resolution_clock::time_point end = high_resolution_clock::now();
    duration<double>  timeRun = duration_cast<duration<double>>(end - start);

    cout << "Наивный алгоритм нашел в тексте " << result.size() << " вхождений" << endl;
    cout << "Время работы Наивного алгоритма: " << std::fixed << timeRun.count() << " секунд" << endl;
}


bool readFile(const string& fileName, string& str) {
    bool isReaded = false;
    string line;

    std::ifstream in;
    in.open(fileName);
    if (in.is_open())
    {
        while (getline(in, line))
        {
            str = str + line + " ";
        }
    }
    isReaded = true;
    in.close();     

    return isReaded;
}

//дальше идут функции для тестирования: чтения файлов с текстом,  деления текста на части (в 4 и 5 тесте мы берем лишь часть текста) и запуска тестов
string getNameFile(int number) {
    switch (number) {
    case 1: return "test1.txt";
    case 2: return "test2.txt";
    case 3: return "test3.txt";
    case 4: return "war3.txt";
    default:
        return "test1.txt";
    }
}

void getPartText(string& text, string& pattern, string& partText, int i) {
    long long lengthText;
    long long lentghPattern;

    switch (i) {
    case 4: {
        lengthText = 5000;
        partText = text.substr(1000, lengthText);
        lentghPattern = lengthText / 4;
        pattern = partText.substr(100, lentghPattern);
        break;
    }
    case 5: {
        lengthText = 10000;
        partText = text.substr(1000, lengthText);
        lentghPattern = lengthText / 4;
        pattern = partText.substr(100, lentghPattern);
        break;
    }
    default:

        break;
    }
}

void run() {
    string text;
    string pattern;
    long long lengthText;
    long long lengthPattern;

    for (int i = 1; i < 4; i++) {
        bool isReaded = readFile(getNameFile(i), text);

        if (isReaded) {
            lengthText = text.length();
            lengthPattern = lengthText / 4;
            pattern = text.substr(0, lengthPattern);

            cout << "Тест  " << i << endl;
            seachKMP(text, pattern);
            rabinKarpSearch(text, pattern);
            search(text, pattern);
            cout << "----------------------------------------------------" << endl;
            text.clear();
            pattern.clear();
        }
    }

    bool isReaded = readFile(getNameFile(4), text);
    string partText;
    if (isReaded) {
        for (int i = 4; i < 6; i++) {
            getPartText(text, pattern, partText, i);
            if (isReaded) {
                cout << "Тест  " << i << std::endl;
                seachKMP(partText, pattern);
                rabinKarpSearch(partText, pattern);
                search(partText, pattern);
                cout << "----------------------------------------------------" << endl;
                partText.clear();
                pattern.clear();
            }
        }

        
        pattern = text.substr(2000, 5000);
         cout << "Тест  " << 6 << endl;        
         seachKMP(text, pattern);
        rabinKarpSearch(text, pattern);
        search(text, pattern);
        cout << "----------------------------------------------------" << endl;
        pattern.clear();
        text.clear();

    }
}

int main() {
    setlocale(LC_ALL, "Russian");

    std::cout << "Лабораторная работа Шарова Иван" << endl;
    run();
}
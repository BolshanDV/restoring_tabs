#include <iostream>
#include "fstream"
#include <vector>
#include <regex>

using namespace std;

string readingFile (){
    ifstream file("/Users/dmitrij/CLionProjects/restoring_tabs1/text.txt");
    string str;
    string content;
    while (getline(file, str)) {
        content += str;
        content.push_back('\n');
    };
    return content;
}

void creatureReadyFile ( const string& text) {
    ofstream out;
    out.open("/Users/dmitrij/CLionProjects/restoring_tabs1/result.txt", ios::app);
    out << text << endl;
};

string& leftTrim(string& str)
{
    string chars = "/ ";

    str.erase(0, str.find_first_not_of(chars));
    return str;
}

vector<string> split (const string& s, const string& delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

struct Nesting {
    vector<string> nestingArr;
    string code;
};

string processForType(string forBlock) {
     forBlock = regex_replace(regex_replace(
            forBlock,
            regex("\n\t"),
            ""),
            regex("\n"),
            "") ;
    string extractedBody = leftTrim(split(forBlock, ")")[1]);
    extractedBody = extractedBody.substr(0, extractedBody.find(';'));
    string type;

    if (extractedBody.find('{') != -1) { type = "ClassicFor"; }
    else if(extractedBody.empty()) { type = "NotValidFor"; }
    else { type = "ForWithoutBrackets"; }
    // if check throw with exception
    return type;
}

string creatingTabsByMain (const string& processedCode){
    vector<string> splitByMain = split(processedCode, "main()");
    splitByMain[0].append(" main()");
    vector<string> bodyMain = split(splitByMain[1], "\n");
    string resultBodyMain;
    for (auto & i : bodyMain) {
        if (i == bodyMain[bodyMain.size() - 2]) {
            i.insert(i.rfind('}'), "\n");
        }
        resultBodyMain.append(i + "\n\t");
    }
    return splitByMain[0] + resultBodyMain;
}
string tabsClassicBodyLines(const string& forBlock) {
    // Double split text
    string splitedText = split(split(forBlock, "{")[1], "}")[0];
    string otherText = forBlock.substr(forBlock.find('}') + 1);
    // Replace all \n and " " to ""
    string condition = forBlock.substr(0, forBlock.find(')') + 1);
    splitedText = regex_replace(regex_replace(splitedText, regex("\n\t"), ""), regex(" "), "");

    // Split elements by ";" and create Vector
    vector<string> body = split(splitedText, ";");

    // Delete last element, because it always empty
    body.pop_back();
    body.push_back(otherText);
    string classicForTabs = condition + "{";
    for (int i = 0; i < body.size() - 1; i++) {
        classicForTabs += "\n\t\t" + body[i];
    }
    classicForTabs += "\n\t}" + body[body.size() - 1];
    return classicForTabs;
}

string tabsWithoutBracketsBodyLines( string forBlock ) {
    string startText = forBlock.substr(0, forBlock.find(')'));
    forBlock.erase(0, forBlock.find(')'));
    string f = forBlock.substr(forBlock.find('\n') - 1 , forBlock.find(';') + 1);
    string conditionWithoutTransfer = regex_replace(regex_replace(regex_replace(
        f.substr(f.find(')') + 1 , f.find(';') + 1),
        regex("\n\t"),
        ""),
        regex("\t"),
        ""),
        regex("\n"),
        ""
        );
    f.replace(forBlock.find(')') , forBlock.find(';') + 1, conditionWithoutTransfer);
    startText = startText + ")\n\t\t" + f;
    string otherText = forBlock.substr(forBlock.find(';') + 1);
    startText.append(otherText);
    return startText;
}
string tabsWithoutBody( string forBlock ) {
    string startText = forBlock.substr(0, forBlock.find(')'));
    forBlock.erase(0, forBlock.find(')'));
    string otherText = forBlock.substr(forBlock.find(';') + 1);
    startText.append( ");\n\t" + otherText);
    return startText;
}

string creatingTabsByBody(const string& body, const string& type) {
    string bodyVector;

    if (type == "ClassicFor") bodyVector = tabsClassicBodyLines(body);
    if (type == "ForWithoutBrackets") bodyVector = tabsWithoutBracketsBodyLines(body);
    if (type == "NotValidFor") bodyVector = tabsWithoutBody(body);

    return bodyVector;
}

string nestingSearch( const string& code, int start, int openBracket) {
    string text = code;
    int num = code.length() - 1;
    int openBracketCount  = openBracket;

    if (start < num) {
        for (int i = start; i < code.length(); i++) {
            if(code[i] == '{') {
                openBracketCount++;
            }
            if(code[i] == '}') {
                openBracketCount--;
            }
            string element = "";
            element = element + code[i];
            string transfer = "";
            transfer = transfer + code[i + 1];

            if(element == "\n"){
                int j;
                int position = i;
                if(transfer == "}") {
                    j = openBracketCount - 1;
                    position += 1;
                } else {
                    j = openBracketCount;
                }
                while (j != 0) {
                    text.insert(i + 1, "\t");
                    position++ ;
                    j--;
                }
                return nestingSearch(text, position, openBracketCount);
            }
        }
    }
    return code;
}

string creatingTabsByFor (const string& processedCode) {
    string resultTabsWithFor;
    vector<string> fors = split(processedCode, "for(");
    resultTabsWithFor.append(fors[0]);

    for (int i = 1; i < fors.size(); i++) {
        string conditionWithoutTransfer = regex_replace(
                fors[i].substr(0, fors[i].find(')') + 1),
                regex("\n\t"),
                "");
        fors[i].replace(0, fors[i].find(')') + 1, conditionWithoutTransfer);

        if(fors[i].substr(fors[i].find(')') + 1, 1) != "\n") {
            fors[i].insert(fors[i].find(')') + 1, "\n");
        }
        // check type for with simple realization
        string type = processForType(fors[i]);

        string resultBody = creatingTabsByBody(fors[i], type);
        resultTabsWithFor.append( "for(" + resultBody);
    }
    return resultTabsWithFor;
}

string creatingTabsByIf ( const string& processedCode, const string& type) {

    string resultTabsWithIf;
    vector<string> splitByIf;
    if (type == "do(") {
        splitByIf = split(processedCode, "do(");
    } else {
        splitByIf = split(processedCode, "if(");
    }

    resultTabsWithIf.append(splitByIf[0]);
    for (int i = 1; i < splitByIf.size(); i++) {
        string finalElement = type + " " + splitByIf[i].substr(0, splitByIf[i].find(')') + 1) + " {";
        string splitedText = split(split(splitByIf[i], "{")[1], "}")[0];
        string bodyWithoutTransfer = regex_replace(regex_replace(regex_replace(
        splitedText,
        regex("\n\t"),""),
        regex("\t"),""),
        regex("\n"),"");
        vector<string> splitBody = split(bodyWithoutTransfer, ";");
        for (int j = 0; j < splitBody.size() - 1; j++ ) {
            finalElement.append("\n\t\t" + splitBody[j] + ";");
        }
        resultTabsWithIf.append(finalElement + "\n\t}");
        resultTabsWithIf.append(splitByIf[i].substr(splitByIf[i].find('}') + 1, splitByIf[i].length()));
    }
    return resultTabsWithIf;
}
Nesting extractNesting( const string& code, vector<string> arr) {
    vector<string> nesting ;
    Nesting mainObj;
    nesting.reserve(arr.size());
    for (const auto & i : arr) {
        nesting.push_back(i);
    }
    string newCode = code;
    int saveStart;
    int saveExit;
    int openBracket = -1;
    int max;
    for (int i = 0; i < code.length(); i++) {
        string type = "";
        type =  type + code[i - 2] + code[i - 1] + code[i];
        if (type == "for") {
            saveStart = i - 2;
        }
        type = code[i];
        if (code[i] == '{') {
            for (int j = i; j < code.length(); j++) {
                switch (code[j]) {
                    case '{':  {
                        openBracket++;
                        if (max < openBracket) {
                            max = openBracket;
                        }
                        break;
                    }
                    case '}': {
                        openBracket--;
                        break;
                    }
                }
                if (openBracket == 0 && max > 1) {
                    saveExit = j + 1;
                    i = j;
                    nesting.push_back(newCode.substr(saveStart, saveExit - saveStart));
                    newCode.replace(saveStart, saveExit - saveStart, "flag");
                    return extractNesting(newCode, nesting);
                } else if (openBracket == 0 && max <= 1){
                    i = j;
                    break;
                }
            }
        }
    };

    mainObj = {nesting, newCode};
    return mainObj;
}
string codeAnalysis(string processedCode) {
    vector<string> typeAnalysis = {"main", "for", "if", "do"};
    for (auto & typeAnalysis : typeAnalysis) {
        if(typeAnalysis == "main") processedCode = creatingTabsByMain(processedCode);
        if( typeAnalysis == "if") processedCode = creatingTabsByIf(processedCode, "if(");
        if( typeAnalysis == "do") processedCode = creatingTabsByIf(processedCode, "do(");
        if( typeAnalysis == "for") processedCode = creatingTabsByFor(processedCode);
    }
    processedCode.insert(processedCode.rfind('}'), "\n");


    return processedCode;
}
string insertCode (const vector<string>& nestingArr, const string& text, int num) {
    vector<string> element = nestingArr;
    string resultCode = text;
    num++;
    for (int i = num; i < element.size(); i++) {
        element[i] = nestingSearch(element[i], 0,1);
        if (text.find("flag")) {
            resultCode.replace(text.find("flag"), 4 , element[i]);
            return insertCode(nestingArr, resultCode, i) ;
        }
    }
    return resultCode;
}
int main() {
    string startingText = readingFile();
    vector<string> element;
    Nesting codeWithExtractElement = extractNesting(startingText, element);
    startingText = codeAnalysis(codeWithExtractElement.code);
    creatureReadyFile(insertCode(codeWithExtractElement.nestingArr, startingText, -1));
    cout << "Success" << endl;
    return 0;
}

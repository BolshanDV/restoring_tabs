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

string processForType(const string& forBlock) {
    string extractedBody = leftTrim(split(forBlock, ")")[1]);

    // check first symbol after "for(cond) { or ; or \n"
    if (extractedBody.find_first_not_of('{')) { return "ClassicFor"; }
    if (extractedBody.find_first_not_of(';')) { return "NotValidFor"; }
    if (extractedBody.find_first_not_of('\n')) { return "ForWithoutBrackets"; }

    // if check throw with exception
    return "Unknown for";
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
vector<string> tabsClassicBodyLines(const string& forBlock) {
    // Double split text
    string splitedText = split(split(forBlock, "{")[1], "}")[0];
    string otherText = forBlock.substr(forBlock.find('}') + 1);
    // Replace all \n and " " to ""

    splitedText = regex_replace(regex_replace(splitedText, regex("\n"), ""), regex(" "), "");

    // Split elements by ";" and create Vector
    vector<string> body = split(splitedText, ";");

    // Delete last element, because it always empty
    body.pop_back();
    body.push_back(otherText);

    return body;
}

string tabsWithoutBracketsBodyLines( string forBlock ) {
    string startText = forBlock.substr(0, forBlock.find(')'));
    string g = forBlock.erase(0, forBlock.find(')'));
    string f = forBlock.substr(forBlock.find('\n') - 1 , forBlock.find(';') + 1);
    string conditionWithoutTransfer = regex_replace(
        f.substr(f.find(')') + 2, f.find(';') + 1),
        regex("\n\t"),
        "");
    f.replace(forBlock.find(')') + 2, forBlock.find(';') + 1, conditionWithoutTransfer);
    startText = startText + f;
    string resultTabsBody = startText.insert(startText.find('\n') + 1, "\t");
    resultTabsBody.insert(resultTabsBody.rfind(';') + 1, "\n");
    return resultTabsBody;
}

string creatingTabsByBody(const string& body, const string& type) {
    string bodyVector;

//    if (type == "ClassicFor") bodyVector = tabsClassicBodyLines(body);
    if (type == "ForWithoutBrackets") bodyVector = tabsWithoutBracketsBodyLines(body);

    return bodyVector;
}

string creatingTabsByFor (const string& processedCode) {
    string resultTabsWithFor;
    vector<string> fors = split(processedCode, "for(");
    resultTabsWithFor.append(fors[0]);

    for (int i = 0; i < fors.size(); i++) {
        // skip i = 0 because it's always not for element
        if (i == 0) continue;

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

string creatingTabsByIf ( const string& processedCode) {
    return "e";
}

string codeAnalysis(string processedCode) {
    vector<string> typeAnalysis = {"main", "for", "if"};
    for (auto & typeAnalysis : typeAnalysis) {
        if(typeAnalysis == "main") processedCode = creatingTabsByMain(processedCode);
        if( typeAnalysis == "for") processedCode = creatingTabsByFor(processedCode);
//        if( typeAnalysis == "if") processedCode = creatingTabsByIf(processedCode);
    }
    return processedCode;
}
int main() {
    string startingText = readingFile();
    creatureReadyFile( codeAnalysis(startingText));
    cout << "Success" << endl;
    return 0;
}

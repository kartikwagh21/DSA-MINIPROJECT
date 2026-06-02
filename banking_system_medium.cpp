#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <fstream>
#include <queue>
#include <stack>

using namespace std;

enum class AccountType { 
    Saving = 1, 
    Current, 
    Business 
};

struct Account { 
    int accountNumber; 
    string holderName; 
    double balance; 
    int pin; 
    AccountType type; 
};

const string JSON_FILE = "accounts.json";

string accountTypeToString(AccountType type) { 
    switch (type) {
        case AccountType::Saving:  return "Saving"; 
        case AccountType::Current: return "Current"; 
        case AccountType::Business: return "Business"; 
        default:                    return "Unknown";
    }
}

static string extractStringValue(const string &obj, const string &key) {
    string pat = '"' + key + '"';
    size_t p = obj.find(pat);
    if (p == string::npos) return "";
    
    p = obj.find(':', p);
    if (p == string::npos) return "";
    p++;
    
    while (p < obj.size() && isspace((unsigned char)obj[p])) p++;
    
    if (p < obj.size() && obj[p] == '"') {
        size_t q = obj.find('"', p + 1);
        if (q == string::npos) return "";
        return obj.substr(p + 1, q - (p + 1));
    }
    return "";
}

static double extractNumberValue(const string &obj, const string &key) {
    string pat = '"' + key + '"';
    size_t p = obj.find(pat);
    if (p == string::npos) return 0.0;
    
    p = obj.find(':', p);
    if (p == string::npos) return 0.0;
    p++;
    
    while (p < obj.size() && isspace((unsigned char)obj[p])) p++;
    
    size_t q = p;
    while (q < obj.size() && (isdigit((unsigned char)obj[q]) || obj[q] == '-' || obj[q] == '.')) q++;
    
    if (q <= p) return 0.0;
    return stod(obj.substr(p, q - p));
}

class MediumBank {
private:
    vector<Account> accounts;
    int nextAccountNumber = 1001;
    stack<pair<int, double>> undoStack;  
    queue<pair<int, string>> serviceQueue; 

    void load() {
        accounts.clear();
        ifstream f(JSON_FILE);
        if (!f) return;
        
        string s, content;
        while (getline(f, s)) {
            content += s;
        }
        f.close();
        
        size_t pos = 0;
        while (true) { 
            size_t l = content.find('{', pos); 
            if (l == string::npos) break; 
            
            size_t r = content.find('}', l); 
            if (r == string::npos) break; 
            
            string obj = content.substr(l, r - l + 1); 
            Account a; 
            
            a.accountNumber = (int)extractNumberValue(obj, "accountNumber"); 
            a.holderName = extractStringValue(obj, "holderName"); 
            a.balance = extractNumberValue(obj, "balance"); 
            a.pin = (int)extractNumberValue(obj, "pin"); 
            
            string tt = extractStringValue(obj, "type"); 
            if (tt == "Saving") {
                a.type = AccountType::Saving; 
            } else if (tt == "Current") {
                a.type = AccountType::Current; 
            } else {
                a.type = AccountType::Business; 
            }
            
            accounts.push_back(a); 
            if (a.accountNumber >= nextAccountNumber) {
                nextAccountNumber = a.accountNumber + 1; 
            }
            pos = r + 1; 
        }
    }

    void save() { 
        ofstream f(JSON_FILE, ios::trunc); 
        if (!f) return; 
        
        f << "[\n"; 
        for (size_t i = 0; i < accounts.size(); ++i) { 
            auto &a = accounts[i]; 
            f << "  {"; 
            f << "\"accountNumber\":" << a.accountNumber << ","; 
            f << "\"holderName\":\"" << a.holderName << "\","; 
            f << "\"balance\":" << fixed << setprecision(2) << a.balance << ","; 
            f << "\"pin\":" << a.pin << ","; 
            f << "\"type\":\"" << accountTypeToString(a.type) << "\""; 
            f << "}"; 
            if (i + 1 < accounts.size()) f << ","; 
            f << "\n"; 
        } 
        f << "]\n"; 
        f.close(); 
    }

public:
    MediumBank() { 
        load(); 
    }

    void createAccount() { 
        Account a{}; 
        a.accountNumber = nextAccountNumber++; 
        
        cout << "Holder Name: "; 
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
        getline(cin, a.holderName); 
        
        cout << "Initial Balance: "; 
        cin >> a.balance; 
        
        cout << "Set PIN: "; 
        cin >> a.pin; 
        
        int choice; 
        cout << "Type (1 Saving, 2 Current, 3 Business): "; 
        cin >> choice; 
        
        if (choice == 1) a.type = AccountType::Saving;
        else if (choice == 2) a.type = AccountType::Current;
        else a.type = AccountType::Business;
        
        accounts.push_back(a); 
        save(); 
        cout << "Account created successfully! Account #: " << a.accountNumber << "\n"; 
    }

    void insertionSortById() { 
        for (int i = 1; i < (int)accounts.size(); ++i) { 
            Account key = accounts[i]; 
            int j = i - 1; 
            while (j >= 0 && accounts[j].accountNumber > key.accountNumber) { 
                accounts[j + 1] = accounts[j]; 
                --j;
            } 
            accounts[j + 1] = key; 
        } 
    }

    int linearSearchById(int accNo) const { 
        for (int i = 0; i < (int)accounts.size(); ++i) {
            if (accounts[i].accountNumber == accNo) {
                return i; 
            }
        }
        return -1; 
    }

    void list() const { 
        if (accounts.empty()) {
            cout << "No accounts found.\n";
            return;
        }
        cout << "\n--- ACCOUNT LIST ---\n";
        for (const auto &a : accounts) {
            cout << "Acc #: " << a.accountNumber 
                 << " | Name: " << a.holderName 
                 << " | Balance: $" << fixed << setprecision(2) << a.balance << "\n"; 
        }
    }

    void withdraw() { 
        int acc; 
        double amt; 
        cout << "Account #: "; 
        cin >> acc; 
        cout << "Amount to Withdraw: "; 
        cin >> amt; 
        
        int idx = linearSearchById(acc); 
        if (idx == -1) { 
            cout << "Error: Account not found.\n"; 
            return; 
        } 
        if (accounts[idx].balance < amt) { 
            cout << "Error: Insufficient balance.\n"; 
            return; 
        } 
        
        accounts[idx].balance -= amt; 
        // Save transaction for undo
        undoStack.push({acc, amt}); 
        save(); 
        cout << "Successfully withdrew $" << amt << "\n"; 
    }

    void undo() { 
        if (undoStack.empty()) { 
            cout << "Nothing to undo.\n"; 
            return; 
        } 
        
        auto p = undoStack.top(); 
        undoStack.pop(); 
        // Use to do Undo operations
        
        int idx = linearSearchById(p.first); 
        if (idx == -1) { 
            cout << "Error: Original account missing.\n"; 
            return; 
        } 
        
        accounts[idx].balance += p.second; 
        save(); 
        cout << "Undo complete. Reversed withdrawal of $" << p.second << "\n"; 
    }

    void enqueueService() { 
        int acc; 
        string svc; 
        cout << "Account #: "; 
        cin >> acc; 
        cout << "Service Type: "; 
        cin >> svc; 
        
        serviceQueue.push({acc, svc}); 
        cout << "Enqueued request to line.\n"; 
    }

    void serve() { 
        if (serviceQueue.empty()) { 
            cout << "No pending clients in the queue.\n"; 
            return; 
        } 
        
        auto p = serviceQueue.front(); 
        serviceQueue.pop(); 
        cout << "Now serving Account #: " << p.first << " for service: [" << p.second << "]\n"; 
    }
};

void showMenu() { 
    cout << "\n===== MEDIUM BANK MENU =====\n"
         << "1. Create Account\n"
         << "2. List Accounts\n"
         << "3. Withdraw\n"
         << "4. Undo Last Withdrawal\n"
         << "5. Enqueue Customer Service\n"
         << "6. Serve Next Customer\n"
         << "7. Exit\n"
         << "----------------------------\n"
         << "Choice: "; 
}

int main() { 
    MediumBank bank; 
    int choice; 
    
    do { 
        showMenu(); 
        if (!(cin >> choice)) {
            cout << "Invalid input. Please enter a number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        
        switch (choice) { 
            case 1: bank.createAccount(); break; 
            case 2: bank.list(); break; 
            case 3: bank.withdraw(); break; 
            case 4: bank.undo(); break; 
            case 5: bank.enqueueService(); break; 
            case 6: bank.serve(); break; 
            case 7: cout << "Goodbye!\n"; break; 
            default: cout << "Invalid option. Try again.\n"; 
        } 
    } while (choice != 7); 
    
    return 0; 

#include<iostream>
#include<map>
#include<vector>
#include<fstream>
#include<sstream>
#include<ctime>
#include "Online.h"
#include "json.hpp"

using namespace std;
using namespace json;

string DateTime(time_t time, const char *format = "%Y-%m-%d %H:%M:%S") {
    char buffer[90];
    struct tm *timeinfo = localtime(&time);
    strftime(buffer, sizeof(buffer), format, timeinfo);
    return buffer;
}

int ShowReadMenu(const vector<string> &choices) {
    int choice;
    while (true) {
        cout << "\nMenu:\n";
        for (int ch = 0; ch < (int) choices.size(); ++ch) {
            cout << "\t" << ch + 1 << ": " << choices[ch] << "\n";
        }
        cout << "Enter number in range " << 1 << " - " << (int) choices.size() << ": ";
        cin >> choice;
        if (choice < 1 || choice > (int) choices.size())
            cout << "Invalid input...\n";
        else
            break;
    }
    return choice;
}

map<string, Book *> Books;
map<string, User *> Users;
//--------------------------------------------------------------------------------------------------------------------
//User

User::User(string username, string name, string password, string email, bool is_admin) {
    setUsername(username);
    setName(name);
    setPassword(password);
    setEmail(email);
    setIsAdmin(is_admin);
}

string User::ToString() const {
    ostringstream oss;
    oss << "Name: " << Name;
    if (IsAdmin)
        oss << " | Admin";
    oss << "\n";
    oss << "Email: " << Email << "\n";
    oss << "User name: " << Username << "\n";
    return oss.str();
}

bool User::getIsAdmin() const {
    return IsAdmin;
}

string User::getPassword() const {
    return Password;
}

string User::getUsername() const {
    return Username;
}

string User::getName() const {
    return Name;
}

string User::getEmail() const {
    return Email;
}

vector<Session *> User::getSessions() { return sessions; }

void User::setSessions(vector<Session *> sessions) {
    this->sessions = sessions;
}

void User::setIsAdmin(bool isAdmin) {
    this->IsAdmin = isAdmin;
}

void User::setPassword(string password) {
    this->Password = password;
}

void User::setUsername(string username) {
    this->Username = username;
}

void User::setName(string name) {
    this->Name = name;
}

void User::setEmail(string email) {
    this->Email = email;
}

void User::clear() { sessions.clear(); }

User::User() {
    setIsAdmin(false);
}

//--------------------------------------------------------------------------------------------------------------------
//UsersManager

UsersManager::UsersManager() : cur_user(nullptr) {}

bool UsersManager::Login(int p) {
    while (true) {
        string username, password;
        cout << "Enter username (no spaces):  (-1 to exit)";
        cin >> username;
        if(username=="-1")
            return false;
        if (!Users.count(username) || Users[username]->getIsAdmin() != p) {
            cout << "No such User.";
            cout << '\n';
            continue;
        }
        cout << "Enter password (no spaces): ";
        cin >> password;
        if (Users[username]->getPassword() != password) {
            cout << "Wrong Credentials...please try again.";
            cout << '\n';
            continue;
        }
        cur_user = Users[username];
        return true;
    }
}

bool UsersManager::SignUp(int p) {
    while (true) {
        string username, password, name, email;
        cout << "Enter username (no spaces):  (-1 to exit)";
        cin >> username;
        if(username=="-1")
            return false;
        if (Users.count(username)) {
            cout << "Taken username...please try again.";
            continue;
        }
        cout << "Enter password (no spaces): ";
        cin >> password;
        cout << "Enter name (no spaces): ";
        cin >> name;
        cout << "Enter email (no spaces): ";
        cin >> email;
        cur_user = new User();
        cur_user->setUsername(username);
        cur_user->setName(name);
        cur_user->setPassword(password);
        cur_user->setEmail(email);
        cur_user->setIsAdmin(p);
        Users[username] = cur_user;
        UpdateDataBase();
        return true;
    }
}

const User *UsersManager::GetCurrentUser() { return cur_user; }

void UsersManager::UpdateDataBase() {
    ofstream file_output("Users.json", ios::trunc);
    if (file_output.fail()) {
        cout << "Can't open the Users output file\n";
        return;
    }
    file_output << "[\n";
    int cnt = 0;
    for (const auto &current_user: Users) {
        User *user = current_user.second;
        JSON obj;
        obj["1-Username"] = user->getUsername();
        obj["2-Password"] = user->getPassword();
        obj["3-Name"] = user->getName();
        obj["4-Email"] = user->getEmail();
        obj["5-Admin"] = (int) user->getIsAdmin();
        file_output << obj;
        if (cnt < (int) Users.size() - 1)
            file_output << ',';
        cnt++;
        file_output << "\n";
    }
    file_output << "]";
    file_output.close();
    ofstream file_output2("Sessions.json", ios::trunc);
    if (file_output2.fail()) {
        cout << "Can't open the Sessions output file\n";
        return;
    }

    file_output2 << "[\n";
    int cnt2 = 0;
    for (const auto &user: Users)
        if (!user.second->getSessions().empty())
            cnt2++;
    for (const auto &user: Users) {
        auto sessions = user.second->getSessions();
        if (sessions.empty())
            continue;
        JSON obj;
        obj["1-Username"] = user.first;
        obj["2-Sessions"] = (int) sessions.size();
        int cnt1 = 1;
        for (auto session: sessions) {
            obj["Session" + to_string(cnt1++)] = session->tostring();
        }
        file_output2 << obj;
        if (cnt2 > 1)
            file_output2 << ',';
        file_output2 << '\n';
        cnt2--;
    }
    file_output2 << "]";
    file_output2.close();
}

void UsersManager::LoadDataBase() {

    ifstream file_input("Users.json");
    if (file_input.fail()) {
        cout << "Can't open the input file\n";
        return;
    }

    string line;
    getline(file_input, line);
    while (true) {
        getline(file_input, line);
        if (line == "]" || line.empty())
            break;
        vector<string> s(5);
        for (int cnt = 0; cnt < 5; cnt++) {
            getline(file_input, line);
            istringstream iss(line);
            string discard;
            iss >> discard >> discard;
            string temp;
            iss >> temp;
            if (cnt < 4) {
                temp.erase(0, 1);
                temp.pop_back();
                temp.pop_back();
            }
            s[cnt] = temp;
        }
        int temp = stoi(s[4]);
        string username = s[0], password = s[1], name = s[2], email = s[3];
        bool is_admin = (temp != 0);
        User *user = new User(username, name, password, email, is_admin);
        Users[username] = user;
        getline(file_input, line);

    }
    if (cur_user)
        cur_user = Users[cur_user->getUsername()];
    file_input.close();
    ifstream file_input2("Sessions.json");
    if (file_input2.fail()) {
        cout << "Can't open the input file\n";
        return;
    }

    getline(file_input2, line);
    while (true) {
        getline(file_input2, line);
        if (line == "]" || line.empty())
            break;
        string username;
        int numberofsessions;
        getline(file_input2, line);
        istringstream iss(line);
        string discard;
        iss >> discard;

        iss >> discard;
        iss >> username;
        username.erase(0, 1);
        username.pop_back();
        username.pop_back();
        getline(file_input2, line);
        istringstream iss2(line);
        iss2 >> discard;
        iss2 >> discard;
        iss2 >> discard;
        discard.pop_back();
        numberofsessions = stoi(discard);
        vector<Session *> s;
        for (int cnt = 0; cnt < numberofsessions; cnt++) {
            getline(file_input2, line);
            istringstream iss3(line);
            iss3 >> discard >> discard >> discard;
            string isbn;
            int idx;
            time_t time;
            iss3 >> isbn >> idx >> time;
            auto session = new Session(isbn, idx, time);
            s.emplace_back(session);
        }
        Users[username]->setSessions(s);
        getline(file_input2, line);
    }
    file_input2.close();
}

void UsersManager::PrintSessions() {

    auto sessions = cur_user->getSessions();
    if (sessions.empty()) {
        cout << "No previous sessions..." << '\n';
        return;
    }
    int counter = 1;
    for (auto cur_session: sessions) {
        cout << counter++ << ": ";
        cur_session->PrintSession();
    }
}

int UsersManager::getSessionsSize() { return (int) (cur_user->getSessions().size()); }

UsersManager::~UsersManager() {
    for (const auto &user: Users) {
        for (const auto &session: user.second->getSessions()) {
            delete session;
        }
        user.second->clear();
        delete user.second;
    }
    Users.clear();
    cur_user = nullptr;
}

void UsersManager::addSessions(vector<Session *> sessions) {
    cur_user->setSessions(sessions);
}

void UsersManager::eraseSessions() {
    cur_user->clear();
}

vector<Session *> UsersManager::getSessions() {
    return cur_user->getSessions();
}
//--------------------------------------------------------------------------------------------------------------------
//Session

Session::Session() : page_idx(0) {}

Session::Session(string isbn, int page_idx, time_t last_access_date) {
    this->book_isbn = isbn;
    this->page_idx = page_idx;
    this->last_access_date = last_access_date;
}

void Session::PrintSession() {
    Book *book = Books[book_isbn];

    cout << book->getTitle() << " Page: " << page_idx << "/" << book->getNumberOfPages() << " - "
         << DateTime(last_access_date) << '\n';
}

string Session::tostring() {
    string temp;
    temp += " " + book_isbn + " ";
    temp += to_string(page_idx) + " ";
    temp += to_string(last_access_date) + " ";
    return temp;
}

const string &Session::getBookIsbn() const {
    return book_isbn;
}

void Session::setBookIsbn(const string &bookIsbn) {
    book_isbn = bookIsbn;
}

int Session::getPageIdx() const {
    return page_idx;
}

void Session::setPageIdx(int pageIdx) {
    page_idx = pageIdx;
}


void Session::setLastAccessDate(time_t lastAccessDate) {
    last_access_date = lastAccessDate;
}


//--------------------------------------------------------------------------------------------------------------------
//BooksManager
void BooksManager::UpdateDataBase() {
    ofstream file_output("Books.json", ios::trunc);
    if (file_output.fail()) {
        cout << "Can't open the output file\n";
        return;
    }
    file_output << "[\n";
    int cnt = 0;
    for (const auto &current_book: Books) {
        Book *book = current_book.second;
        JSON obj;
        obj["1-Title"] = book->getTitle();
        obj["2-ISBN"] = book->getIsbn();
        obj["3-Author"] = book->getAuthor();
        obj["4-Pages"] = book->getNumberOfPages();


        vector<string> pages = book->getPages();
        for (int page = 0; page < (int) (book->getNumberOfPages()); page++) {
            obj["Page" + to_string(page + 1)] = pages[page];
        }
        if (cnt != (int) Books.size() - 1) {
            file_output << obj << ",\n";
        } else
            file_output << obj << "\n]";
        cnt++;
    }
    file_output.close();
}

void BooksManager::LoadDataBase() {
    ifstream file_input("Books.json");
    if (file_input.fail()) {
        cout << "Can't open the input file\n";
        return;
    }
    string line;
    getline(file_input, line);
    while (true) {
        string title, author;
        int number_of_pages;
        string ISBN;
        getline(file_input, line);
        if (line == "]" || line.empty())
            break;
        vector<string> s(4);
        for (int cnt = 0; cnt < 4; cnt++) {
            getline(file_input, line);
            istringstream iss(line);
            string discard;
            iss >> discard >> discard;
            string temp;
            iss >> temp;
            if (cnt < 3) {
                temp.erase(0, 1);
                temp.pop_back();
            }
            temp.pop_back();
            s[cnt] = temp;
        }
        int temp = stoi(s[3]);
        tie(title, ISBN, author, number_of_pages) = tie(s[0], s[1], s[2], temp);

        vector<string> pages(number_of_pages);
        for (int i = 0; i < number_of_pages; i++) {
            getline(file_input, line);
            if (i != number_of_pages - 1)
                line.pop_back();
            line.pop_back();
            string page = line.substr(line.find_last_of('"') + 1);
            pages[i] = page;
        }

        Book *book = new Book(ISBN, title, author, number_of_pages, pages);

        Books[ISBN] = book;
        getline(file_input, line);
    }

    file_input.close();
}

void BooksManager::AddBook() {
    string ISBN;
    string title, author;
    int number_of_pages;
    cout << "Enter ISBN: ";
    cin >> ISBN;
    cout << "Enter Title: ";
    cin >> title;
    cout << "Enter Author Name: ";
    cin >> author;
    cout << "Enter How many pages: ";
    cin >> number_of_pages;
    vector<string> pages(number_of_pages);
    for (int page = 0; page < number_of_pages; page++) {
        cout << "Enter Page #" << page + 1 << ": ";
        if (page == 0)
            getline(cin, pages[page]);
        getline(cin, pages[page]);
    }

    Book *book = new Book();
    book->setIsbn(ISBN);
    book->setNumberOfPages(number_of_pages);
    book->setTitle(title);
    book->setAuthor(author);
    book->setPages(pages);
    Books[ISBN] = book;
    UpdateDataBase();
}
string BooksManager::PrintBooks() {
    vector<string> book_titles, book_isbn;
    if (Books.empty()) {
        cout << "No books available.\n";
        return "-1";
    }
    int counter = 1;
    for (const auto &current: Books) {
        Book *current_book = current.second;
        book_titles.emplace_back(current_book->getTitle());
        book_isbn.emplace_back(current_book->getIsbn());
        cout << counter << ": " << current_book->getTitle()<<"  ISBN:" << current_book->getIsbn()<<'\n';
        counter++;
    }
    int index;
    while(true)
    {
        cout << "Enter book number:";
        cin>>index;
        if(index<1||index>book_titles.size())
            cout<<"Invalid Input..\n";
        else
            break;
    }
    return book_isbn[index-1];
}
void BooksManager::DeleteBook() {
    string isbn=PrintBooks();
    if(isbn=="-1") { cout << "No Books available\n";return; }
    if (Books.count(isbn))
        Books.erase(isbn);
    else
        cout << "No such book ISBN" << "\n";
    UpdateDataBase();
}

void BooksManager::UpdateBook() {
    string isbn=PrintBooks();
    if(isbn=="-1") { cout << "No Books available\n";return; }
    if (!Books.count(isbn))
        cout << "No such book ISBN" << "\n";
    else {
        Book *book = Books[isbn];
        vector<string> s;
        s.emplace_back("Update Title.");
        s.emplace_back("Update Author.");
        s.emplace_back("Update Pages.");
        s.emplace_back("Add Pages.");
        s.emplace_back("Return");
        int choice = ShowReadMenu(s);
        if (choice == 5)
            return;
        if (choice == 1) {
            string title;
            cout << "Enter new title:";
            cin >> title;
            book->setTitle(title);
        } else if (choice == 2) {
            string author;
            cout << "Enter new author:";
            cin >> author;
            book->setAuthor(author);
        } else if(choice==3){
            vector<string> pages = book->getPages();
            int pageidx;
            cout << "Enter index of the page to be updated:";
            cin >> pageidx;
            string page;
            cout<<"Enter Page content:";
            getline(cin, page);
            getline(cin, page);
            pages[pageidx - 1] = page;
            book->setPages(pages);
        }
        else
        {
            vector<string> pages = book->getPages();
            int number_of_pages;
            cout << "Enter How many pages: ";
            cin >> number_of_pages;
            for (int page = 0; page < number_of_pages; page++) {
                cout << "Enter Page #" << (int)pages.size()+page + 1 << ": ";
                string temp;
                if (page == 0)
                    getline(cin, temp);
                getline(cin, temp);
                pages.push_back(temp);
            }
            book->setNumberOfPages((int)pages.size());
            book->setPages(pages);
        }
        UpdateDataBase();
    }

}
//--------------------------------------------------------------------------------------------------------------------
//UserView

UserView::UserView(UsersManager &usersManager, BooksManager &booksManager) : users_manager(usersManager),
                                                                             books_manager(booksManager) {
}

void UserView::Display() {
    const User *user = users_manager.GetCurrentUser();
    cout << "\n\nHello " << user->getName() << " | User View\n";

    vector<string> menu;
    menu.push_back("View Profile");
    menu.push_back("List & Select from My Reading History");
    menu.push_back("List & Select from Available Books");
    menu.push_back("Logout");

    while (true) {

        users_manager.LoadDataBase();
        books_manager.LoadDataBase();

        int choice = ShowReadMenu(menu);
        if (choice < 1 || choice > 4)
            continue;
        if (choice == 1)
            ViewProfile();
        else if (choice == 2)
            ListHistory();
        else if (choice == 3)
            ListBooks();
        else
            break;
    }
}

void UserView::ListHistory() {


    int counter = users_manager.getSessionsSize();
    users_manager.PrintSessions();
    if ((int) users_manager.getSessionsSize() == 0)
        return;
    int index;
    while (true) {
        cout << "Enter number in range 1 - " + to_string(users_manager.getSessionsSize()) << "\n";
        cin >> index;
        if (index <= (int) users_manager.getSessionsSize() && index >= 1)
            break;
        cout << "Invalid Input\n";
    }
    Session *cur_session = users_manager.getSessions()[index - 1];
    Book *cur_book = Books[cur_session->getBookIsbn()];
    Session *new_session = cur_book->Read(cur_session->getPageIdx());
    vector<Session *> sessions = users_manager.getSessions();
    sessions.erase(sessions.begin() + index - 1);
    sessions.emplace_back(new_session);
    users_manager.eraseSessions();
    users_manager.addSessions(sessions);
    users_manager.UpdateDataBase();
}

void UserView::ListBooks() {
    vector<string> book_titles, book_isbn;
    if (Books.empty()) {
        cout << "No books available.\n";
        return;
    }
    int counter = 1;
    for (const auto &current: Books) {
        Book *current_book = current.second;
        book_titles.emplace_back(current_book->getTitle());
        book_isbn.emplace_back(current_book->getIsbn());
        cout << counter << ": " << current_book->getTitle() << '\n';
        counter++;
    }
    int index;
    index = ShowReadMenu(book_titles);
    Book *book = Books[book_isbn[index - 1]];
    Session *new_session = book->Read();
    auto sessions = users_manager.getSessions();
    users_manager.eraseSessions();
    sessions.emplace_back(new_session);
    users_manager.addSessions(sessions);
    users_manager.UpdateDataBase();
}

void UserView::ViewProfile() {
    const User *user = users_manager.GetCurrentUser();
    cout << "\n" << user->ToString() << "\n";
}

//--------------------------------------------------------------------------------------------------------------------
AdminView::AdminView(UsersManager &usersManager, BooksManager &booksManager) : users_manager(usersManager),
                                                                               books_manager(booksManager) {}

void AdminView::Display() {
    const User *user = users_manager.GetCurrentUser();

    cout << "\n\nHello " << user->getName() << " | Admin View\n";
    vector<string> menu;
    menu.push_back("View Profile");
    menu.push_back("Add Book");
    menu.push_back("Delete Book");
    menu.push_back("Update Book");
    menu.push_back("Logout");

    while (true) {
        users_manager.LoadDataBase();
        books_manager.LoadDataBase();
        int choice = ShowReadMenu(menu);
        if (choice == 1)
            ViewProfile();
        else if (choice == 2)
            AddBook();
        else if (choice == 3)
            DeleteBook();
        else if (choice == 4)
            UpdateBook();
        else
            break;
    }
}

void AdminView::ViewProfile() {
    const User *user = users_manager.GetCurrentUser();

    cout << "\n" << user->ToString() << "\n";
}

void AdminView::AddBook() {
    books_manager.AddBook();
}

void AdminView::DeleteBook() {
    books_manager.DeleteBook();
}

void AdminView::UpdateBook() {
    books_manager.UpdateBook();
}

//-------------------------------------------
//Book

Book::Book() : NumberOfPages(0) { setNumberOfPages(0); }

Book::Book(const string &ISBN, const string &title, const string &author, int number_of_pages,
           const vector<string> &pages) {
    setIsbn(ISBN);
    setTitle(title);
    setAuthor(author);
    setNumberOfPages(number_of_pages);
    setPages(pages);
}

string Book::getIsbn() const {
    return ISBN;
}

Session *Book::Read(int index) {
    while (true) {
        cout << "Current Page: " << index << "/" << NumberOfPages << '\n';
        cout << Pages[index - 1] << '\n';
        vector<string> s;
        s.push_back("1: Next Page");
        s.push_back("2: Previous Page");
        s.push_back("3: Stop Reading");
        int choice = ShowReadMenu(s);
        if (choice == 1)
            index = min(index + 1, NumberOfPages);
        else if (choice == 2)
            index = max(1, index - 1);
        else
            break;
    }
    time_t t = time(nullptr);
    auto session = new Session(ISBN, index, t);
    return session;
}

void Book::setIsbn(string isbn) {
    ISBN = isbn;
}

const string &Book::getTitle() const {
    return Title;
}

void Book::setTitle(const string &title) {
    Title = title;
}

const string &Book::getAuthor() const {
    return Author;
}

void Book::setAuthor(const string &author) {
    Author = author;
}

int Book::getNumberOfPages() const {
    return NumberOfPages;
}

void Book::setNumberOfPages(int numberOfPages) {
    NumberOfPages = numberOfPages;
}

const vector<std::string> &Book::getPages() const {
    return Pages;
}

void Book::setPages(const vector<std::string> &pages) {
    Pages = pages;
}
//------------------------------------------------

int OnlineBookReader::AdminOrCustomer() {

    vector<string> s;
    s.push_back("Admin");
    s.push_back("Customer");
    s.push_back("Exit.");
    int choice = ShowReadMenu(s);
    return choice;
}

int OnlineBookReader::LoginOrSignUp() {

    int choice;
    vector<string> s;
    s.push_back("Login");
    s.push_back("Sign Up");
    s.push_back("Return");
    choice = ShowReadMenu(s);
    return choice;
}

OnlineBookReader::OnlineBookReader() : books_manager(new BooksManager()), users_manager(new UsersManager()) {}

OnlineBookReader::~OnlineBookReader() {
    cout << "OnlineReaderSystem  Shutdown\n";
    Clear();
    if (books_manager != nullptr) {



        delete books_manager;
        books_manager = nullptr;
    }
    if (users_manager != nullptr) {

        delete users_manager;

        users_manager = nullptr;

    }
}

void OnlineBookReader::LoadDataBase() {
    users_manager->LoadDataBase();
    books_manager->LoadDataBase();
}

void OnlineBookReader::Clear() {
    for (auto book: Books) {
        delete book.second;
    }
    Books.clear();
}

void OnlineBookReader::Start() {

    while (true) {

        LoadDataBase();
        int choice1 = AdminOrCustomer();
        if (choice1 == 3) {
            Clear();
            break;
        }
        int choice2 = LoginOrSignUp();
        if (choice2 == 3)
            continue;
        int p = 0;
        if (choice1 == 1)
            p = 1;
        if (choice2 == 1)
        {if(!users_manager->Login(p))continue;}
        else { if(!users_manager->SignUp(p))continue;}
        if (choice1 == 1) {

            AdminView adminView = AdminView(*users_manager, *books_manager);
            adminView.Display();
        } else {

            UserView userView = UserView(*users_manager, *books_manager);
            userView.Display();
        }


    }
}
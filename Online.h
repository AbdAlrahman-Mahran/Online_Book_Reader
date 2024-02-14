//
// Created by AbdAlrahman on 10/02/2024.
//
#ifndef ONLINE_ONLINE_H
#define ONLINE_ONLINE_H
#include<map>
#include<vector>

class User;
class Book;
class Session;
class OnlineBookReader;
extern std::map<std::string,Book*> Books;
extern std::map<std::string,User*> Users;
class User{
private:
    std::string Name;
    std::string Username;
    std::string Email;
    std::string Password;
    bool IsAdmin;
    std::vector<Session*> sessions;
public:
    User();
    User(std::string,std::string,std::string,std::string,bool);
    User(const User&) = delete;
    void operator=(const User&) = delete;
    std::string ToString() const;
    std::string getPassword()const;
    std::string getUsername()const;
    std::string getName()    const;
    std::string getEmail( )  const;
    bool getIsAdmin()const;
    std::vector<Session*> getSessions();

    void clear();
    void addSessions();
    void eraseSessions();
    void setSessions(std::vector<Session*>);
    void setIsAdmin(bool);
    void setPassword(std::string);
    void setUsername(std::string);
    void setName(std::string);
    void setEmail(std::string);
};

class Book{
private:
    std::string ISBN;
    std::string Title;
    std::string Author;
    int NumberOfPages;
    std::vector<std::string> Pages;
public:
    Book();
    Book(const std::string& ,const std::string&,const std::string&,int,const std::vector<std::string>&);

    Session* Read(int index=1);

    std::string getIsbn() const;

    void setIsbn(std::string);

    const std::string &getTitle() const;

    void setTitle(const std::string &);

    const std::string &getAuthor() const;

    void setAuthor(const std::string &);

    int getNumberOfPages() const;

    void setNumberOfPages(int);

    const std::vector<std::string> &getPages() const;

    void setPages(const std::vector<std::string> &);

};

class Session
{
private:
    std::string book_isbn;
    int page_idx;
    time_t last_access_date;
public:
    Session();
    Session(std::string, int, time_t);

    const std::string &getBookIsbn() const;

    void setBookIsbn(const std::string &bookIsbn);

    int getPageIdx() const;

    void setPageIdx(int pageIdx);


    void setLastAccessDate(time_t lastAccessDate);

    void PrintSession();
    std::string tostring();
};
class UsersManager{
private:
    User* cur_user;
public:
    UsersManager();
    ~UsersManager();
    UsersManager(const UsersManager&) = delete;
    void operator=(const UsersManager&) = delete;
    void LoadDataBase();
    void UpdateDataBase();
    void PrintSessions();
    int getSessionsSize();
    std::vector<Session*> getSessions();
    void eraseSessions();
    void addSessions(std::vector<Session*>);
    bool Login(int p);
    bool SignUp(int p=0);
    const User* GetCurrentUser();
};
class BooksManager
{
public:
    void LoadDataBase();
    void UpdateDataBase();
    void AddBook();
    std::string PrintBooks();
    void UpdateBook();
    void DeleteBook();
};
class UserView{
private:
    UsersManager& users_manager;
    BooksManager& books_manager;
public:
    UserView(UsersManager&,BooksManager&);
    void Display();
    void ViewProfile();
    void ListHistory();
    void ListBooks();
};

class AdminView
{
private:
    UsersManager& users_manager;
    BooksManager& books_manager;

public:
    AdminView(UsersManager&,BooksManager&);
    void Display();
    void ViewProfile();
    void DeleteBook();
    void UpdateBook();
    void AddBook();
};
class OnlineBookReader{
private:
    UsersManager* users_manager;
    BooksManager* books_manager;
public:
    OnlineBookReader();
    ~OnlineBookReader();
    void LoadDataBase();
    void Clear();
    int AdminOrCustomer();
    int LoginOrSignUp();
    void Start();
};

#endif //ONLINE_ONLINE_H

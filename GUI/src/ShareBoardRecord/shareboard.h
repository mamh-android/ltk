#ifndef SHAREBOARD_H
#define SHAREBOARD_H
class QString;
enum SQLShareRecord{SQLBoard_id,SQLStartTime,SQLUserName,SQLUserTeam,SQLIpaddr,SQLMachine};

class shareboard{
public:
    shareboard();
    ~shareboard();
    bool createConnection();
    int share(const QString &board_id, const QString &startTime,  const QString &userName,  const QString &userTeam, const QString &ipaddr,const QString &machine);
    int unshare(const QString &board_id, const QString &startTime, const QString &endTime);
    int addNewRowSQL(const QString board_id, const QString startTime, const QString userName, const QString userTeam, const QString ipaddr, const QString machine);

private:
     static QString MySQLDataBaseName;


};
#endif // SHAREBOARD_H

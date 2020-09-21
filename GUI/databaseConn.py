#Импорт библиотеки для работы с СУБД PostgreSQL
import psycopg2
from psycopg2 import sql

class dbConn():

#Инициализация подключения\\\\\\\\\\\\\\\\\\\\\\
    def __init__(self, User, Password):         
        self.conn = psycopg2.connect(
            dbname = "SpeechCorpus",
            user = User,
            password = Password
        )

        self.conn.set_isolation_level(0)
        self.cursor = self.conn.cursor()        

#Стандартный запрос на поиск\\\\\\\\\\\\\\\\\\\\
    def select(self, table, field, whereLike, value):       
        self.cursor.execute(sql.SQL("SELECT {field} FROM speech_base.{table} {whereLike} {value}").format(
            table = sql.SQL(table),             
            field = sql.SQL(field),             
            whereLike = sql.SQL(whereLike),     
            value = sql.SQL(value)))                        
    #Результаты
        rows = self.cursor.fetchall()      
        return rows

#Отключение от базы\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
    def DBConnClose(self):
        self.cursor.close()
        self.conn.close()

#Поиск и выдача файла по названию\\\\\\\\\\\\\\\
    def selectSpeech(self, file, check):
        row = self.select('"speech"', '"filename", "speaker_id", "text_id"', 'WHERE "filename" LIKE', "'"+file+"%'")

        results = ['','','']
        results[0] = 'Нет результатов'
        for r in row:
            results[0] = 'E:/1SpeechComplex/DB/data' + r[0]
            results[1] = r[1]
            results[2] = r[2]
        row = self.select('"speaker"', '"name", "family_name"', 'WHERE cast("speaker_id" as text) LIKE', "'"+str(results[1])+"'")
        
        for r in row:
            results[1] = r[0] + ' ' + r[1]
        if(check == 0):
            row = self.select('"text"', '"spelling_record"', 'WHERE cast("text_id" as text) LIKE', "'"+str(results[2])+"'")
        else:
            row = self.select('"text"', '"transcription"', 'WHERE cast("text_id" as text) LIKE', "'"+str(results[2])+"'")
        for r in row:
            results[2] = r[0]

        return results

#Проверка роли\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
    def currentRole(self, username):
        self.cursor.execute("SELECT has_function_privilege(%s, 'speech_base.user_registr(character varying, character varying)', 'execute')",[username])
        row = self.cursor.fetchall() 
        for r in row:
            userrol = r[0]
        return userrol

#Вставка\\\\\\\\\\\\\\\\\\
    def write(self, table, fields, values):
        self.cursor.execute(sql.SQL("INSERT INTO speech_base.{table} ({fields}) VALUES ({values})").format(
            table = sql.SQL(table),             
            fields = sql.SQL(fields),                  
            values = sql.SQL(values)))

#Поиск и выдача файла по названию\\\\\\\\\\\\\\\
    def selectAdmin(self, table, field, value):
        row = self.select(table, '*', 'WHERE cast("'+field+'" as text) LIKE', value)

        results = ''
        for r in row:
            for i in r:
                results = results + str(i) + '\n'
        if (results == ''):
            results = 'Нет результатов'

        return results

#Создание юзера\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
    def addTester(self, username, password):
        self.cursor.execute(sql.SQL("CREATE USER {name} WITH PASSWORD '{password}'; GRANT tester TO {name};").format(
            name = sql.SQL(username),
            password = sql.SQL(password)))

#Обновление\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
    def update(self, table, field, value, id, idnum):
        self.cursor.execute(sql.SQL("UPDATE speech_base.{table} SET {field} = {value} WHERE {id} = {idnum}").format(
            table = sql.SQL(table),             
            field = sql.SQL(field),                  
            value = sql.SQL(value),
            id = sql.SQL(id),                  
            idnum = sql.SQL(idnum)))

#Удаление\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
    def delete(self, table, field, value):
        self.cursor.execute(sql.SQL("DELETE FROM speech_base.{table} WHERE {field} = {value}").format(
            table = sql.SQL(table),             
            field = sql.SQL(field),                  
            value = sql.SQL(value)))
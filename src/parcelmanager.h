#ifndef PARCELMANAGER_H
#define PARCELMANAGER_H

#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>

struct Parcel {
    QString id;
    QString number;
    QString company;
    QString alias;

    QJsonObject toJson() const;
    static Parcel fromJson(const QJsonObject& obj);
};

class ParcelManager {
public:
    ParcelManager();

    void addParcel(const Parcel& parcel);
    void removeParcel(const QString& id);
    QVector<Parcel> getAllParcels() const;
    Parcel getParcelById(const QString& id) const;

    bool loadFromFile(const QString& filePath);
    bool saveToFile(const QString& filePath);

private:
    QVector<Parcel> parcels;
};

#endif // PARCELMANAGER_H

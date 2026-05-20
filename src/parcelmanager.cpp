#include "parcelmanager.h"
#include <QJsonDocument>
#include <QFile>
#include <QDir>

QJsonObject Parcel::toJson() const {
    QJsonObject obj;
    obj["id"] = id;
    obj["number"] = number;
    obj["company"] = company;
    obj["alias"] = alias;
    return obj;
}

Parcel Parcel::fromJson(const QJsonObject& obj) {
    Parcel p;
    p.id = obj["id"].toString();
    p.number = obj["number"].toString();
    p.company = obj["company"].toString();
    p.alias = obj["alias"].toString();
    return p;
}

ParcelManager::ParcelManager() {
}

void ParcelManager::addParcel(const Parcel& parcel) {
    parcels.append(parcel);
}

void ParcelManager::removeParcel(const QString& id) {
    for (int i = 0; i < parcels.size(); ++i) {
        if (parcels[i].id == id) {
            parcels.removeAt(i);
            break;
        }
    }
}

QVector<Parcel> ParcelManager::getAllParcels() const {
    return parcels;
}

Parcel ParcelManager::getParcelById(const QString& id) const {
    for (const auto& p : parcels) {
        if (p.id == id) {
            return p;
        }
    }
    return Parcel();
}

bool ParcelManager::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) {
        return false;
    }

    parcels.clear();
    QJsonArray arr = doc.array();
    for (const auto& val : arr) {
        if (val.isObject()) {
            parcels.append(Parcel::fromJson(val.toObject()));
        }
    }
    return true;
}

bool ParcelManager::saveToFile(const QString& filePath) {
    QDir dir(QFileInfo(filePath).dir());
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QJsonArray arr;
    for (const auto& p : parcels) {
        arr.append(p.toJson());
    }

    QJsonDocument doc(arr);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(doc.toJson());
    file.close();
    return true;
}

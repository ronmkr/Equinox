#include "ProfileManager.h"

namespace equinox
{

ProfileManager::ProfileManager()
{
    auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                        .getChildFile("Equinox");
    appDataDir.createDirectory();
    dbFile = appDataDir.getChildFile("profiles.xml");
    
    initDatabase();
}

void ProfileManager::initDatabase()
{
    if (!dbFile.exists())
    {
        juce::ValueTree root("EquinoxProfiles");
        auto xml = root.createXml();
        xml->writeTo(dbFile);
    }
}

bool ProfileManager::saveProfile(const EqProfile& profile)
{
    auto xml = juce::XmlDocument::parse(dbFile);
    if (!xml) return false;

    auto root = juce::ValueTree::fromXml(*xml);
    
    // Find existing or create new
    auto existing = root.getChildWithProperty("name", profile.name);
    if (existing.isValid()) {
        root.removeChild(existing, nullptr);
    }

    juce::ValueTree newProfile("Profile");
    newProfile.setProperty("name", profile.name, nullptr);
    newProfile.setProperty("preamp", profile.preampDb, nullptr);
    
    juce::String gainsStr;
    for (auto g : profile.gains) gainsStr += juce::String(g) + ",";
    newProfile.setProperty("gains", gainsStr, nullptr);

    root.addChild(newProfile, -1, nullptr);
    
    auto finalXml = root.createXml();
    return finalXml->writeTo(dbFile);
}

EqProfile ProfileManager::loadProfile(const juce::String& name)
{
    EqProfile profile;
    profile.name = name;
    profile.gains.assign(31, 0.0f);

    auto xml = juce::XmlDocument::parse(dbFile);
    if (!xml) return profile;

    auto root = juce::ValueTree::fromXml(*xml);
    auto p = root.getChildWithProperty("name", name);

    if (p.isValid())
    {
        profile.preampDb = (float)p.getProperty("preamp");
        juce::StringArray gains;
        gains.addTokens(p.getProperty("gains").toString(), ",", "");
        for (int i = 0; i < std::min(31, gains.size()); ++i)
            profile.gains[i] = gains[i].getFloatValue();
    }

    return profile;
}

juce::StringArray ProfileManager::getAllProfileNames()
{
    juce::StringArray names;
    auto xml = juce::XmlDocument::parse(dbFile);
    if (xml)
    {
        auto root = juce::ValueTree::fromXml(*xml);
        for (int i = 0; i < root.getNumChildren(); ++i)
            names.add(root.getChild(i).getProperty("name").toString());
    }
    return names;
}

bool ProfileManager::deleteProfile(const juce::String& name)
{
    auto xml = juce::XmlDocument::parse(dbFile);
    if (!xml) return false;

    auto root = juce::ValueTree::fromXml(*xml);
    auto p = root.getChildWithProperty("name", name);
    if (p.isValid())
    {
        root.removeChild(p, nullptr);
        auto finalXml = root.createXml();
        return finalXml->writeTo(dbFile);
    }
    return false;
}

} // namespace equinox

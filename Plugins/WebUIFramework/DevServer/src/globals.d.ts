declare global {
    interface Window {
        // 'ue' is our Game Instance. Its functions are exposed directly.
        ue: {
            // WARNING: These signatures MUST match the C++ UFUNCTIONs.
            StartNewGame: (levelName: string) => void;
            QuitGame: () => void;
            RequestSaveGamesList: () => void;
            LoadGame: (slotName: string) => void;
            SaveGame: (slotName: string) => void;
            ApplyAllGameSettings: (settingsJson: string) => void;
        };

        // This function is called BY C++ to give us the save data.
        populateSaveSlots: (saveData: any[]) => void;
    }
}
export {};
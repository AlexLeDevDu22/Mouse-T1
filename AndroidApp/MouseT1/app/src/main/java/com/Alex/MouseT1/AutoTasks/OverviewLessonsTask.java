package com.Alex.MouseT1.AutoTasks;

import static com.Alex.MouseT1.OtherFun.sendLargeJson;

import android.app.Activity;
import android.os.Build;
import android.util.Log;

import androidx.annotation.RequiresApi;


import java.io.File;
import java.nio.file.Files;
import java.nio.file.Paths;

public class OverviewLessonsTask extends Activity {
    @RequiresApi(api = Build.VERSION_CODES.O)
    public void startTask(Activity activity, String matiere, String postUrl) {
        try {
            String lessonList = getLessonsBySubject(activity, matiere);

            String postLessons;
            assert lessonList != null;
            if (!lessonList.equals("[]")) {
                postLessons = "{\"type\":\"overview\",\"response\":\"overview lessons\",overview:" + lessonList + "}";
            } else {
                postLessons = "{\"type\":\"text\",\"text\":\"No lesson\"}";
            }
            if (postLessons.length()>2048){
                postLessons="{\"type\":\"text\",\"text\":\"Erreur: Trop de leçons!\"}";
            }

            Log.d("testaa","sending: "+postLessons);
            sendLargeJson(postLessons, postUrl);
        }catch(Exception e){
            Log.e("testaa",e.toString());
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    private String getLessonsBySubject(Activity activity, String subject) {
        try {
            StringBuilder filteredLessons = new StringBuilder("[");

            File filesDir = activity.getExternalFilesDir(null);
            File[] files = filesDir != null ? filesDir.listFiles((dir, name) -> name.endsWith(".txt")) : null;

            if (files != null) {
                for (File file : files) {
                    File metadataFile = new File(file.getPath().replace(".txt", ".meta"));
                    String metaSubject;
                    metaSubject = new String(Files.readAllBytes(Paths.get(metadataFile.getPath())));

                    if (metaSubject.equals(subject)) {
                        //ajt a la list
                        filteredLessons.append("{'text':'").append(file.getName().replaceAll(".txt$","")).append("'},");
                    }
                }
                filteredLessons = new StringBuilder(filteredLessons.toString().replaceAll(",$", "") + "]");

                return filteredLessons.toString();

            }
        }catch (Exception e) {
            Log.e("testaa",e.toString());
        }
        return null;
    }
}
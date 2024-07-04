package com.Alex.MouseT1.UI

import android.annotation.SuppressLint
import android.os.Bundle
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.Alex.MouseT1.OtherFun
import com.Alex.MouseT1.R
import java.io.File
import java.text.SimpleDateFormat
import java.util.*

class LessonTextActivity : AppCompatActivity() {

    @SuppressLint("SetTextI18n")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_lesson_text)

        val lessonName = intent.getStringExtra("lessonName") ?: ""
        val lessonSubject = intent.getStringExtra("lessonSubject") ?: "Inconnue"
        val lessonDate = intent.getLongExtra("lessonDate", 0L)

        // Charger le texte du fichier basé sur le nom du fichier
        val file = File(getExternalFilesDir(null), lessonName)
        val lessonText = OtherFun.readTextFromFile(file)

        // Afficher le contenu dans une TextView par exemple
        val lessonNameTextView = findViewById<TextView>(R.id.lessonNameTextView)
        val lessonSubjectTextView = findViewById<TextView>(R.id.lessonSubjectTextView)
        val lessonDateTextView = findViewById<TextView>(R.id.lessonDateTextView)
        val lessonContentTextView = findViewById<TextView>(R.id.lessonContentTextView)

        lessonNameTextView.text = "Nom: $lessonName"
        lessonSubjectTextView.text = "Matière: $lessonSubject"
        lessonDateTextView.text = "Date: " + SimpleDateFormat("dd/MM/yyyy", Locale.getDefault()).format(Date(lessonDate))
        lessonContentTextView.text = lessonText
    }
}

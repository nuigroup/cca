/***************************************************************************
*
*  ofxSphinxASR.cpp
* 
*  Author: Jimbo Zhang <dr.jimbozhang@gmail.com>
*  Copyright 2010 Jimbo Zhang. All rights reserved.       
*
*
* This file is part of ofxASR, an openframeworks addon for speech
* recognition.
*
* ofxASR is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Community Core Audio is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with Community Core Audio.  If not, see <http://www.gnu.org/licenses/>.
*
*
***************************************************************************/

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "ofxSphinxASR.h"

ofxSphinxASR::ofxSphinxASR()
{
    ofAddListener(ofEvents.audioReceived, this, &ofxSphinxASR::_audioReceived);
    bEngineInitialed = false;
    bEngineStarted = false;
    decoder = NULL;
}

ofxSphinxASR::~ofxSphinxASR()
{

}

int ofxSphinxASR::engineInit(ofAsrEngineArgs *e)
{
    char cfg_filename[] = "sphinx.cfg";
    char grammarJSGF_filename[] = "grammar.jsgf";
    char grammarFSG_filename[] = "grammar.fsg";
    FILE *cfg_fp = fopen(cfg_filename, "wt");
    if (cfg_fp==NULL)
        return OFXASR_FAIL_WRITE_CONFIG;

    if ( access(e->sphinxmodel_am.c_str(), 0) != 0 ||
         access(e->sphinxmodel_lm.c_str(), 0) != 0 ||
         access(e->sphinxmodel_dict.c_str(), 0) != 0 ||
         access(e->sphinxmodel_fdict.c_str(), 0) != 0 ) {
        return OFXASR_FAIL_READ_FILES;
    }

    fprintf(cfg_fp, "-samprate %d\n", e->samplerate);
    fprintf(cfg_fp, "-hmm %s\n", e->sphinxmodel_am.c_str());
    fprintf(cfg_fp, "-dict %s\n", e->sphinxmodel_dict.c_str());
    fprintf(cfg_fp, "-fdict %s\n", e->sphinxmodel_fdict.c_str());
    fprintf(cfg_fp, "-lm %s\n", e->sphinxmodel_lm.c_str());

    if (e->sphinx_mode == 2) {
        if(e->sphinx_candidate_sentences.size() < 1) {
            printf("Warning: The word list has not been provided! Use mode 0.\n");
            e->sphinx_mode = 0;
        }
        else {
            FILE *gram_fp = fopen(grammarJSGF_filename, "wt");
            if (gram_fp==NULL)
                return OFXASR_FAIL_WRITE_CONFIG;

            fprintf(gram_fp, "#JSGF V1.0;\n\ngrammar cca_gram;\n\npublic <cca_gram> = (\n");
            for (int i=0; i<e->sphinx_candidate_sentences.size()-1; i++) {
                fprintf(gram_fp, "%s |\n", e->sphinx_candidate_sentences[i].c_str());
            }
            fprintf(gram_fp, "%s );\n\n", 
                e->sphinx_candidate_sentences[e->sphinx_candidate_sentences.size()-1].c_str());
            fclose(gram_fp);
        }

        jsgf_t *jsgf = jsgf_parse_file(grammarJSGF_filename, NULL);
        if (jsgf == NULL) {
            return OFXASR_INVAILED_JSGF_GRAMMAR;
        }
        fsg_model_t *fsg = get_fsg(jsgf, NULL);
        fsg_model_writefile(fsg, grammarFSG_filename);
        fsg_model_free(fsg);
        jsgf_grammar_free(jsgf);
    }

    fprintf(cfg_fp, "-op_mode %d\n", e->sphinx_mode);
    fclose(cfg_fp);
    
    bEngineInitialed = true;

    return OFXASR_SUCCESS;
}

int ofxSphinxASR::engineExit()
{
    if (decoder != NULL) {
        delete decoder;
        decoder = NULL;
    }

    return OFXASR_SUCCESS;
}

int ofxSphinxASR::engineOpen()
{
    if (! bEngineInitialed) 
        return OFXASR_HAVE_NOT_INIT;

    bEngineStarted = true;
    return OFXASR_SUCCESS;
}

int ofxSphinxASR::engineClose()
{
    if (! bEngineInitialed) 
        return OFXASR_HAVE_NOT_INIT;

    bEngineStarted = false;
    return OFXASR_SUCCESS;
}

int ofxSphinxASR::engineReset()
{

    return OFXASR_SUCCESS;
}

int ofxSphinxASR::engineSentAudio(char *audioBuf, int audioSize)
{

    return OFXASR_SUCCESS;
}

char * ofxSphinxASR::engineGetText()
{
    return NULL;
}

void ofxSphinxASR::_audioReceived(ofAudioEventArgs &e)
{

}

bool ofxSphinxASR::isEngineStarted()
{
    return bEngineInitialed * bEngineStarted;
}

fsg_model_t* ofxSphinxASR::get_fsg(jsgf_t *grammar, const char *name)
{
    jsgf_rule_iter_t *itor;
    logmath_t *lmath = logmath_init(1.0001, 0, 0);
    fsg_model_t *fsg = NULL;

    for (itor = jsgf_rule_iter(grammar); itor;
        itor = jsgf_rule_iter_next(itor)) {
            jsgf_rule_t *rule = jsgf_rule_iter_rule(itor);
            char const *rule_name = jsgf_rule_name(rule);

            if ((name == NULL && jsgf_rule_public(rule))
                || (name && strlen(rule_name)-2 == strlen(name) &&
                0 == strncmp(rule_name + 1, name, strlen(rule_name) - 2))) {
                    fsg = jsgf_build_fsg_raw(grammar, rule, logmath_retain(lmath), 1.0);
                    jsgf_rule_iter_free(itor);
                    break;
            }
    }

    logmath_free(lmath);
    return fsg;
}

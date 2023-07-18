from sentence_transformers import SentenceTransformer

model = SentenceTransformer('sentence-transformers/paraphrase-MiniLM-L3-v2')#all-MiniLM-L6-v2')

import time

t0 = None

def start_timer():
    global t0
    t0 = time.time()
    
def end_timer():
    t1 = time.time()
    
    print('took', t1 - t0, 'seconds')

def flatten(l):
    return [item for sublist in l for item in sublist]

def remove_blank(parts):
    return list(filter(lambda x: len(x) > 0, parts))

from heapq import heapify, heappush, heappushpop, nlargest

class MaxHeap():
    def __init__(self, top_n):
        self.h = []
        self.length = top_n
        heapify( self.h)
        
    def add(self, element):
        if len(self.h) < self.length:
            heappush(self.h, element)
        else:
            heappushpop(self.h, element)
            
    def getTop(self):
        return sorted(self.h, reverse=True)

import re

def split_into_sentences(paragraph):
    punct_regex = r"(?=\S)(?:[A-Z][a-z]{0,3}\.|[^.?!;:]|\.(?!\s+[A-Z]))*.?"
    return re.findall(punct_regex, paragraph)

def split_clauses(text):
    return re.findall(r'(.*?(?:(?:, )|—|$))', text)

def swap(c):
    return '"' if c == "'" else "'"

def split_paren_quotes(text):
    # very simple algorithm, assuming it's balanced
    # only capture outer groups, since our algorithm below is sequential (does not take
    # recursive relations into account and simply concatenates string sections)
    count_paren = 0
    count_quot = 0
    curr_quot = "'"
    #wait_until_whitespace = False
    
    strings = []
    curr_str = ""
    for c in text:
        prev_sum = count_paren + count_quot
        if c == ')':
            count_paren -= 1
        elif c == '(':
            count_paren += 1
        elif c == '"' or c == "'":
            # you can't have nested " marks, unless you have '
            if curr_quot == c:
                count_quot -= 1
                curr_quot = swap(curr_quot)
            else:
                count_quot += 1
                curr_quot = swap(curr_quot)
        new_sum = count_paren + count_quot
        
        '''if wait_until_whitespace:
            if c.isspace():
                strings.append((curr_str, False))
                curr_str = ""
                curr_str += c
                wait_until_whitespace = False
            else:
                curr_str += c
        el'''
        if prev_sum != 0 and new_sum == 0:
            curr_str += c
            strings.append((curr_str, False))
            curr_str = ""
            #wait_until_whitespace = True
        elif prev_sum == 0 and new_sum != 0:
            strings.append((curr_str, True))
            curr_str = ""
            curr_str += c
        else:
            curr_str += c
    
    if len(curr_str) > 0:
        strings.append((curr_str, True))
    
    return strings

def split(text):
    sections = split_paren_quotes(text)
    clauses = flatten([split_clauses(s) if b else [s] for s, b in sections])
    
    # todo: split clauses into sections with parentheses, sections with quotes, etc. 
    
    return remove_blank(clauses)

from sentence_transformers.util import cos_sim

def find_semantic(prompt, parts: list, ret_only_best=True):
    start_timer()
    prompt_embedding = model.encode(prompt, convert_to_tensor=True)
    frag_embeddings = model.encode(parts, convert_to_tensor=True)
    end_timer()
    
    top_k_ret = []
    
    i = 0
    for f in parts:
        top_k_ret.append((f, cos_sim(prompt_embedding, frag_embeddings[i])))
        i += 1
        
    top_k_ret.sort(key=lambda x: -x[1])
    
    if ret_only_best:
        return top_k_ret[0][0]
    
    return [c[0] for c in top_k_ret]

def get_best_par(prompt, section, attenuation_factor=0.9):
    start_timer()
    
    paragraphs = section.split('\n')
    paragraphs = remove_blank(paragraphs)
    
    paragraphs = list(reversed(paragraphs[-3:]))
    
    prompt_embedding = model.encode(prompt, convert_to_tensor=True)
    par_embeddings = model.encode(paragraphs, convert_to_tensor=True)
    
    # find which paragraph to search in -- only check last 3
    # unfortunately, we have to choose just one paragraph to serach in for performance reasons
    # remember that this server is more powerful than most consumer machines, and also
    # that we're *only* running this algorithm, not an additional voice to text + user application
    # + presumably a word processor where this output is going
    # in the future we might be able to consider all fragments from the latest k paragraphs
    # and simply attenuate those cosine similarities
    m = len(paragraphs)
    
    best_par = None
    best_sim = -1
    
    count = 0
    for i in range(0, min(3, len(paragraphs))):
        sim = cos_sim(prompt_embedding, par_embeddings[i]) * (attenuation_factor ** count)
        if best_par is None or sim > best_sim:
            best_sim = sim
            best_par = paragraphs[i]
        count += 1
    
    end_timer()
    
    return best_par

def search_frag(prompt, section, attenuation_factor=0.9):
    best_par = get_best_par(prompt, section, attenuation_factor)
    
    # TODO: figure out how many fragments the average sentence has
    # there are up to O(n^2) fragments, since a fragment is a contiguous subsection defined
    # by all pairs of indices
    
    # split into sentences. then, split into fragments
    sentences = split_into_sentences(best_par)
    fragments = flatten([split(s) for s in sentences])
    
    n = len(fragments)
    
    frag_batches = []
    
    for l in range(1, n + 1):
        for start in range(n - l + 1):
            end = start + l
            
            frag = ''.join(fragments[start:end])
            frag_batches.append(frag)
    
    return find_semantic(prompt, frag_batches, ret_only_best=False)

from thefuzz import fuzz, process

from difflib import SequenceMatcher as SM
from nltk.util import ngrams
import codecs

# return top 3 -- use fuzzywuzzy library
def find_exact_fuzzy_par(prompt, part, top_k=4):
    needle = prompt
    hay    = part

    needle_length  = len(needle.split())
    max_sim_val    = 0
    max_sim_string = u""

    top_k = MaxHeap(top_k)

    for ngram in ngrams(hay.split(), needle_length + int(.2*needle_length)):
        hay_ngram = u" ".join(ngram)
        similarity = SM(None, hay_ngram, needle).ratio()
        top_k.add((similarity, hay_ngram))

    return top_k.getTop()

def find_exact_fuzzy(prompt, section, top_k=3):
    paragraphs = section.split('\n')
    paragraphs = remove_blank(paragraphs)
    
    paragraphs = list(reversed(paragraphs[-3:]))
    
    matches = []
    
    i = 0
    for p in paragraphs:
        for sim, text in find_exact_fuzzy_par(prompt, p):
            matches.append((sim, -i, text))
        i += 1
    
    matches.sort()
    
    return list(reversed(matches[-top_k:]))

# note: IMPORTANT!!!
# we choose these hyperparameters because:
# * exact matches tend to either get it, or they don't -- especially if you're not trying to
#   find an exact match verbally
# * when the top exact match isn't what you're looking for, worse matches are likely to contain
#   the same contiguous subsection as the best match. so, no point in displaying too many of them
# * semantic matches tend to be more varied -- include more
# * but, semantic matches also suffer from similar issues with repeated substrings -- not necessarily
#   a bad thing, but because of this, it's good to limit how many we display
# * also good to balance finding matches with information overload

def get_best_matches(prompt, text, top_ex_fuzz=2, top_sem=3, sem_par_atten=0.9):
    start_timer()
    ex_fuzz = find_exact_fuzzy(prompt, text, top_ex_fuzz)
    end_timer()
    sem = search_frag(prompt, text, sem_par_atten)
    
    ret = []
    
    for score, par, text in ex_fuzz:
        ret.append(text)
    
    for i in range(min(top_sem, len(sem))):
        ret.append(sem[i])
    
    return ret

prepositions = set(['of', 'with', 'on', 'that', 'about', 'containing', 'regarding'])

def classify_type(prompt: str):
    first_words = prompt.split(' ', 3)
    if 'sentence' in first_words[:2]:
        return ('sentence', first_words[-1])
    elif 'paragraph' in first_words[:2]:
        return ('paragraph', first_words[-1])
    elif 'section' in first_words[:2]:
        return ('best_match', first_words[-1])
    else:
        return ('best_match', prompt)

def smart_matches(prompt, text):
    """
    Smartly determines the type of query, and generates best matches. 
    Uses sensible defaults for hyperparameters. 
    If this function determines that the prompt is asking for a
    sentence or paragraph rather than a fragment, it will also return
    fragments just in case. 
    
    Return order:
    best exact/fuzzy matches given prompt
    best exact/fuzzy fragments
    
    best semantic matches given prompt
    best semantic fragments
    """
    
    parts = []
    
    query_type, new_prompt = classify_type(prompt)
    
    if query_type == 'sentence':
        parts = split_into_sentences(text)
    elif query_type == 'paragraph':
        parts = text.split('\n')
        parts = remove_blank(parts)

        parts = list(reversed(parts[-3:]))
    else:
        return get_best_matches(new_prompt, text, top_ex_fuzz=3, top_sem=5)
    
    start_timer()
    top_k = MaxHeap(2)

    for p in parts:
        similarity = SM(None, p, new_prompt).ratio()
        top_k.add((similarity, p))
    
    ex_fuzz = top_k.getTop()
    end_timer()
    start_timer()
    sem = find_semantic(new_prompt, parts, ret_only_best=False)
    end_timer()
        
    # choosing top 4 and top 7 has no impact on performance, but also
    # allows us to guarantee that these matches are different from the
    # ones we already found
    frag_matches = get_best_matches(prompt, text, top_ex_fuzz=4, top_sem=7)
    
    ret = []
    
    count = 0
    for score, text in ex_fuzz:
        ret.append(text)
        count += 1
    
    for text in frag_matches[:4]:
        if text not in ret:
            ret.append(text)
            count += 1
        
        if count >= 3:
            break
    
    count = 0
    for s in sem:
        if s in ret:
            continue
        ret.append(s)
        count += 1
        
        if count >= 4:
            break
        
    for text in frag_matches[4:]:
        if text not in ret:
            ret.append(text)
            count += 1
        
        if count >= 5:
            break
    
    print('---')
    
    return ret

"""
Set up server
"""

import socket

HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 5010  # Port to listen on (non-privileged ports are > 1023)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen()

while True:
    conn, addr = s.accept()

    try:
        print(f"Connected by {addr}")
        data_parts = []
        while True:
            data = conn.recv(1024)
            if not data:
                break

            print(data)

            latest = data.decode()

            data_parts.append(latest)

            if latest.endswith('\0\0\0'):
                # we have received the whole message
                print('three nulls')
                msg = " ".join(data_parts)
                parts = msg.split('\0')
                text = parts[0].strip().strip('\0')
                prompt = parts[1].strip().strip('\0')

                print('text:', text)
                print('prompt:', prompt)

                data_parts.clear()
                
                matches = smart_matches(prompt, text)

                for m in matches:
                    print('match:', m)
                    conn.send((m + '\0').encode())
    except:
        print('connection closed or other error occurred')
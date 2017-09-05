#! /usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import unicode_literals

import bs4 # python-beautifulsoup4
import difflib

# python-selenium
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.common.action_chains import ActionChains

backend = 'html.parser'
# backend = 'lxml'

context_size = 3

# Suppress exceptions whenever possible
exp = BaseException

def ortho(text):
    diff, full, hints = (None, None, None)
    try:
        full, hints = query(text)
        diff = wdiff(text, full)
    except exp:
        return None

    if (diff is None or len(diff) < 1) and (hints is None or len(hints) < 1):
        return None

    return (diff, full, hints)

def wdiff(orig, revd):
    diff = difflib.unified_diff(orig.split(), revd.split())
    s = ''
    n = False
    p = False
    c = context_size
    r = False
    for d in diff:
        if d.startswith(('---', '+++', '@@')):
            continue
        fd = d[1:] + ' '
        if d.startswith('-'):
            if p:
                s += '} -{' + fd
                p = False
                n = True
            elif n:
                s += fd
            else:
                if not r:
                    r = True
                    s += '« '
                s += ' -{' + fd
                n = True
        elif d.startswith('+'):
            if n:
                s += '} +{' + fd
                n = False
                p = True
            elif p:
                s += fd
            else:
                if not r:
                    r = True
                    s += '« '
                s += ' +{' + fd
                p = True
        else:
            if p:
                s += '} '
                p = False
                if not n:
                    c = 0
            elif n:
                s += '} '
                n = False
                if not p:
                    c = 0

            if c < context_size:
                if not r:
                    r = True
                    s += '« '
                s += fd
                c += 1
            elif r:
                r = False
                s += ' », '
    if p or n:
        s += '}'
    if r:
        s += ' », '

    while '  ' in s:
        s = s.replace('  ', ' ')
    s = s.replace('{ ', '{').replace(' }', '}')
    if s.endswith(' '):
        s = s[:-1]
    if s.endswith(','):
        s = s[:-1]

    if len(s) > 2:
        return '*' + s
    return None

def query(text):
    driver = webdriver.PhantomJS() # phantomjs
    driver.implicitly_wait(0.1) # don't hesitate

    corrected = cordial(driver, text)
    if corrected is None:
        corrected = text

    hints = bonpatron(driver, corrected)

    driver.quit()
    return (corrected, hints)

def cordial(driver, text):
    try:
        driver.get('http://www.cordial-enligne.fr/')
        driver.find_element_by_class_name('cordial-textarea').click()
        ActionChains(driver).key_down(Keys.CONTROL
            ).send_keys('a'
            ).key_up(Keys.CONTROL
            ).send_keys(Keys.DELETE
            ).send_keys(text
            ).perform()
    except exp:
        return text

    driver.find_element_by_class_name('btnCordialBleu').click()
    try:
        result = WebDriverWait(driver, 10).until(
            EC.visibility_of_element_located(
                (By.ID, 'satisfaction'))
            )
        result = driver.find_element_by_class_name('retourCorrect')
    except exp:
        return text

    soup = bs4.BeautifulSoup(result.get_attribute('innerHTML'), backend)
    return soup.text[:-6]

def bonpatron(driver, text):
    formats = [
        ('<i>', '« '),
        ('</i>', ' »'),
        ('<em>', '« '),
        ('</em>', ' »'),
        ('<b>', '['),
        ('</b>', ']'),
        ('<strong>', '['),
        ('</strong>', ']')]

    try:
        driver.get('http://bonpatron.com')
        ActionChains(driver).key_down(Keys.CONTROL
            ).send_keys('a'
            ).key_up(Keys.CONTROL
            ).send_keys(Keys.DELETE
            ).send_keys(text
            ).perform()
    except exp:
        return []

    driver.find_element_by_class_name('mainbutton').click()
    try:
        result = WebDriverWait(driver, 1).until(
            EC.visibility_of_element_located(
                (By.ID, 'summarycontainer'))
            )
    except exp:
        return []

    tips = []
    soup = bs4.BeautifulSoup(result.get_attribute('innerHTML'), backend)
    for li in soup.findAll('li'):
        if not ('id' in li.attrs and li['id'].startswith('summary-')):
            continue

        name = li.span.string
        text = li.span['title']
        for f in formats:
            text = text.replace(*f)
        tips.append((name, text))

    return tips

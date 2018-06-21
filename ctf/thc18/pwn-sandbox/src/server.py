import subprocess
import hashlib
import stat
import time
import os

import web

urls = ('/', 'upload')
header = '<html><head></head><body>'
content = '''
              <div style="padding: 1em;">
                <h2>Upload your binaries into the sandbox here:</h2>
                <form method="POST" enctype="multipart/form-data" action="">
                  <input type="file" name="binfile" />
                  <input type="submit" value="submit"/>
                </form>
              </div>
          '''
answer = '''
              <div style="padding: 1em;">
                <h2>Result:</h2>
                <pre>{}</pre>
              </div>
         '''
entry = '{}:\n    {}\n\n'
timing = 'Executed during {}s...'
footer = '</body></html>'

class upload:
    def GET(self):
        web.header("Content-Type", "text/html; charset=utf-8")
        return header + content + footer

    def POST(self):
        try:
            rq = web.input(binfile={})
            if 'binfile' in rq:
                filepath = rq.binfile.filename.replace('\\','/')
                filename = filepath.split('/')[-1]
                hashname = hashlib.sha224(bytes(filename, 'utf8')).hexdigest()

                path = os.path.join('.', 'binaries', hashname)
                with open(path, 'xb') as f:
                    f.write(rq.binfile.file.read())

                st = os.stat(path)
                os.chmod(path, st.st_mode | stat.S_IEXEC)

                rcode = 0
                start = time.time()
                stdout = b'(no output)'
                try:
                    stdout = subprocess.check_output(
                        ['/usr/bin/ldd', path],
                        stderr=subprocess.STDOUT)

                    if not b'/libc.so.6' in stdout:
                        raise RuntimeError('libc not found')

                    env = os.environ.copy()
                    env["LD_PRELOAD"] = os.path.join(*
                        [os.getcwd(), 'sandkox', 'sandkox-preload.so'])

                    process = subprocess.Popen(
                        [path],
                        env=env,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)

                    while process.returncode is None:
                        process.poll()
                        if time.time() - start > 10:
                            process.kill()
                            break

                    stdout = str(process.stdout.read(), 'utf8')[:-1]
                    stderr = str(process.stderr.read(), 'utf8')[:-1]
                    rcode = process.returncode
                except RuntimeError as e:
                    stdout = str(stdout, 'utf8')
                    stderr = str(e)
                    rcode = -1
                except subprocess.CalledProcessError as e:
                    stdout = str(e.output, 'utf8')
                    stderr = str(e)
                    rcode = e.returncode

                os.remove(path)

                stdout = stdout.replace('\n', '\n    ')
                stderr = stderr.replace('\n', '\n    ')

                stdout = stdout.replace(hashname, filename)
                stderr = stderr.replace(hashname, filename)

                return (header
                    + content
                    + answer.format(''
                        + entry.format('fingerprint', hashname)
                        + entry.format('stdout', stdout)
                        + entry.format('stderr', stderr)
                        + entry.format('exit code', rcode)
                        + timing.format(time.time() - start))
                    + footer)

        except BaseException as e:
            try:
                os.remove(path)
            except BaseException:
                pass

            return header + content + answer.format(str(e)) + footer

if __name__ == "__main__":
   app = web.application(urls, globals())
   app.run()
